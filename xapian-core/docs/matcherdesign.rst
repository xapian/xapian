.. |->| unicode:: U+2192 .. right arrow

Matcher Design Notes
====================

This document was substantially overhauled in April 2022.  We think it
is complete and accurate (at least for the state of things at that point)
but if you find parts that are missing, unclear, or apparently wrong please
report them so we can improve it.

The PostList class
------------------

This is essentially an iterator over a monotonically increasing list of
document ids (which are unsigned integers from 1 upwards).

The iterator starts just before the start of the list (unlike STL-style
iterators which start on the first entry).

``PostList::at_end()`` tests if the iterator has reached the end, and
isn't valid until the iterator has been advanced.

``PostList::next(min_weight)`` advances to the next entry.

``PostList::skip_to(did, min_weight)`` advances to the entry with docid
did, or if that isn't present the next entry after it, or the end of
iteration if there are no entries after it.

``PostList::check(did, min_weight, valid)`` is a variant of ``skip_to()``
and the default implementation of it is a thin wrapper which calls
``skip_to()`` and sets ``valid`` (which is passed by reference) to ``true``.
This default implementation is appropriate if handling the non-matching case
by finding the next docid after the requested that can be done at little extra
cost (e.g. if trying to advance to did naturally leaves us on the first entry
after it when did isn't present).  However if finding the next entry could be
expensive (e.g. for a positional filter which would need to keep checking
documents until it found one which matched) then if did isn't present it sets
valid to false and leaves the position unspecified, except that the
next call to ``next()`` will advance to the first matching document
after ``did``, and ``skip_to()`` will act as it would if the position was the
first matching position after ``did``.

Callers should only call ``skip_to()`` if they actually need to end up on an
entry, and ``check()`` for cases where they just want to check if an entry
is present.

The methods ``next()``, ``skip_to()`` and ``check()`` all return a
``PostList*``.  See the section on "Restructuring Optimisations" below for
details of how this is used.

The PostList Tree
-----------------

The QueryOptimiser class (with help from LocalSubMatch) builds a tree structure
of PostList objects from the query.  When searching multiple shards there's
a separate such tree for each shard.  For a remote shard this tree build and
the execution of it is performed on the remote and a serialised MSet object is
sent across the link, which is then merged with a single MSet object from any
local shards plus MSet objects from other remote shards.  If there's only a
single shard and it is remote then the MSet object from it is simply
unserialised and returned.

At the leaf level, a PostList object is created for each term, and for other
leaf-level subqueries such as PostingSource objects and value ranges. Then
pairs or groups of PostLists are combined using 2-way or n-way branching tree
elements for AND, OR, etc - these are PostList subclasses whose class names
reflect the operation (AndPostList, OrPostList, etc). See below for a full
list.

Running the Match
-----------------

Once the tree is built, the matcher repeatedly asks the root of the tree
for the next matching document.  It maintains a "proto-mset" which
contains the current candidate results.

There are two phases to forming the proto-mset.

During the first phase, results from the postlist tree are simply appended.
This phase continues until the proto-mset contains first+maxitems entries.  If
the postlist tree runs out before this happens, the second phase is skipped
entirely.

During the second phase, the postlist tree operates a one-in, one-out policy.
At the start of this phase, the proto-mset is formed into a min-heap - the heap
ordering function is such that the lowest ranking document in the heap is at
the tip (that ranking may be by relevance weight, or by the sort order if
sorting is used).

Each new candidate is compared to the current tip of the heap - if the
candidate ranks higher then the tip of the heap is replaced with the new
candidate and the heap invariant maintained (the heap implementation we use has
a special operation for "replace tip of heap").

When the postlist tree finally runs out, nth_element is used to eliminate
any results ranking above first (if first is non-zero) and the remaining
documents are sorted.

PostList subclasses
-------------------

There are many subclasses of PostList.  Most can be used in a weighted or
unweighted (boolean) context - the only difference is whether the weights are
used or not.  Sometimes there's a specialised subclass for the unweighted
context (e.g. BoolOrPostList).

LeafPostList
~~~~~~~~~~~~

This is a subclass of PostList which serves as a base class for postlists
for terms and postlists matching all documents in the shard (which is used
for MatchAll and also in some cases when the query optimiser can determine a
subquery matches all documents).  Don't be mislead by the name - it isn't
actually used for all types of leaf query.

ContiguousAllDocsPostList
'''''''''''''''''''''''''

This is a subclass of LeafPostList which matches all documents in the case
where a shard has all docids in use contiguous from 1 upwards.  It's especially
efficient as internally it just needs to count from 1 to n.

GlassPostList, etc
''''''''''''''''''

Each database backend provides a LeafPostList subclass for term subqueries.

HoneyPosPostList
''''''''''''''''

The honey backend's has a special subclass of HoneyPostList for when positional
information is needed to avoid the space overhead of support for positional
information when it isn't needed.  Older backends just always include support
for positional information in e.g. GlassPostList.

GlassAllDocsPostList, etc
'''''''''''''''''''''''''

Each database backend provides a LeafPostList subclass matching all documents,
for use in the general case where ContiguousAllDocsPostList can't be used.
Typically this works from the document length data, which is cache friendly
in as most weighting schemes use document length.

Logical Operators
~~~~~~~~~~~~~~~~~

These implement logical operations.  In most cases they also calculate
relevance weights.

OrPostList
''''''''''

Each OrPostList has two children and returns documents matching either, with
the weight returned for a document being the sum of the weights from the
children which match that document.

An OR operation with more than two subqueries results in a subtree of
OrPostList objects.  This tree is built up in a similar way to how an optimal
huffman code is constructed, so the sub-PostLists with the fewest entries are
furthest down the tree, and those with most nearest the top.

OrPostList is coded for maximum efficiency when the right branch has fewer
postings in than the left branch, and the tree is built that way.

This form of tree is more efficient than a naively balanced tree in terms
of the number of comparisons which need to be performed, ignoring various
optimisations which the matcher can perform.  It's possible that an N-way
OrPostList using a heap or similar would actually do a better job in practice.
We have an N-way BoolOrPostList which is specialised for an unweighted OR
and gets used for expanded wildcards, below SynonymPostList, on the right side
of AndNotPostList and AndMaybePostList, etc.

When a child of OrPostList runs out, the child's at\_end method returns true
and the OrPostList autoprunes, replacing itself with the other child (if both
children run out simultaneously this still happens and the parent of the
OrPostList then handles at\_end from the other child) - see the section on
autopruning below for more details.

OrPostList can decay to AndMaybePostList or AndPostList.  See the section
on operator decay below.

BoolOrPostList
''''''''''''''

This is a multi-way version of OrPostList for use in an unweighted context.
It helps to accelerate wildcard queries for example.

Once all but one child has ended, BoolOrPostList autoprunes and replaces itself
with that child (if all remaining children run out simultaneously this still
happens with an arbitrary child replacing the BoolOrPostList, and the parent of
the BoolOrPostList then handles at\_end from that child).

Because of how BoolOrPostList is weighted, operator decay isn't possible.

MaxPostList
'''''''''''

MaxPostList is like OrPostList except the weight returned for a document is the
maximum weight of any of its matching children rather than the sum of those
weights.

Once all but one child has ended, MaxPostList autoprunes and replaces itself
with that child (if all remaining children run out simultaneously this still
happens with an arbitrary child replacing the MaxPostList, and the parent of
the MaxPostList then handles at\_end from that child).

Because of how MaxPostList is weighted, operator decay isn't possible.

AndPostList
'''''''''''

AndPostList handles a multi-way AND operation, returning documents which match
all its children, summing weights from all children.

When checking for a match, it tries the sub-postlists in order from least
frequent to most frequent.  This will tend to minimise the number of posting
list entries we read and maximise the size of each skip\_to.

When a child of the AndPostList runs out, its at_end method returns true and
the AndPostList's at\_end method will then return true too.

The OP\_FILTER query operator is actually treated as AND in the postlist
tree - the boolean-ness is pushed down to the leaf level below the OP\_FILTER
by setting factor to 0, which is handled by not setting a Weight object.

AndMaybePostList can decay to AndPostList.  See the section on operator decay
below.

XorPostList
'''''''''''

XorPostList handles a multi-way XOR operation, returning documents which match
an odd number of its children and summing weights from the matching children.

Once all but one child has ended, XorPostList autoprunes and replaces itself with
that child (if all remaining children run out simultaneously this still happens
with an arbitrary child replacing the XorPostList, and the parent of the
XorPostList then handles at\_end from that child).

We could decay XorPostList to AndNotPostList in some situations, but we don't
currently attempt to perform this optimisation.

AndNotPostList
''''''''''''''

AndNotPostList returns documents which match its left branch, but not its
right.  The weight returned is just that of its left branch (weights of
documents from the right branch are ignored).

"X ANDNOT Y" implements what search engines commonly support as "X -Y" in their
query syntax.

If the right side runs out first, the AndNotPostList autoprunes and replaces
itself with its left side.  If the left side runs out first, then at\_end
method returns true.

AndMaybePostList
''''''''''''''''

AndMaybePostList returns documents which match the left branch with weights
added on from the right branch if that also matches (so an unweighted
AndMaybePostList isn't useful and the right branch gets discarded in this
situation).

"X ANDMAYBE Y" implements what search engines commonly offer as "+X Y" in their
query syntax.

If the right side runs out first, the AndMaybePostList autoprunes and replaces
itself with its left side.  If the left side runs out first, then at\_end
method returns true.

OrPosPostList
~~~~~~~~~~~~~

OrPosPostList is added above an OR below a positional operator and implements
read\_position\_list() returning the merge of positions from its descendants.

ExternalPostList
~~~~~~~~~~~~~~~~

This is a leaf postlist (but not a LeafPostList!) which calls a PostingSource.

ExtraWeightPostList
~~~~~~~~~~~~~~~~~~~

If the weighting scheme has a term-independent weight contribution (i.e.
get\_maxextra() returns a value > 0) then this is added to the top of the
postlist tree to add on this extra contribution.

SelectPostList
~~~~~~~~~~~~~~

This is a parent class for filtering postlists - it's used for positional
filters and DeciderPostList.

WrapperPostList
~~~~~~~~~~~~~~~

This is a parent class for postlists which provide some sort of "add-on"
to a child postlist.  It's really just an implementation detail to aid sharing
of code.

DeciderPostList
~~~~~~~~~~~~~~~

If a MatchDecider is in use, a DeciderPostList is added at the top of the
postlist tree to implement filtering of returned results.

Positional Operators
--------------------

The positional Query operators are OP\_PHRASE and OP\_NEAR.

The way these are implemented is to perform an AND query for all the terms,
with a filter PostList in front which only returns documents whose positional
information fulfils the phrase requirements.  There's NearPostList for
OP\_NEAR, while OP\_PHRASE has PhrasePostList and also ExactPhrasePostList
which handles the common special-case where the phrase window size is the
same as the number of terms in the phrase.

Checking the positional information is usually expensive compared to matching
postlist trees, so we hoist the position check higher up the tree in cases when
the phrase operation is below an AND or AND-like operation.  For example,
A AND (B NEAR C) will actually filter the results of (A AND B AND C) through a
check for B NEAR C, which means we never need to check positions for documents
which don't match A.

The AND-like operations which can be combined in this way are those inside
OP\_NEAR and OP\_PHRASE, OP\_AND itself, OP\_FILTER, and the left sides of
OP\_AND\_MAYBE and OP\_AND\_NOT.

The most general structure of a sub-tree from such hoisting looks like::

  ...->AndMaybe->{ExactPhrase|Phrase|Near}->...->AndNot->And(...)
          \                                          \
           ->Or(...)                                  ->BoolOr(...)

The AndMaybe and AndNot parts may not be present, and there may be any number
(including zero) of positional filters.

The positional filter check is generally much more expensive than the other
operators involved here, so AndNot is placed below the positional filters so it
can eliminate candidates without having to perform positional checks on them.
On the other hand, AndMaybe only potentially adds weight, so it's placed above
the positional filters as that way we can at least avoid checking its right
side for documents which are rejected by a positional filter.

Restructuring Optimisations
---------------------------

There are two restructuring optimisations which can happen during the matching
process: autoprune and operator decay.

Autoprune
~~~~~~~~~

For example, if a branch in the match tree is "A OR B", when A runs out then
"A OR B" is replaced by "B".  Which postlists this can happen for is documented
above.

The mechanism by which autoprune happens is that the next or skip\_to method
returns a non-NULL pointer, which is the PostList object.  The caller should
delete the PostList object it called the method on and replace it with the
returned object.

Operator Decay
~~~~~~~~~~~~~~

The lowest weight currently needed make the proto-mset decreases
monotonically as the match progresses which the matcher passes into the
PostList tree as a parameter when it calls next().  Each PostList also
reports a maximum weight it could contribute, and each node uses this
to adjust the minimum weight it passes on to its children.

For example, if the left branch of an AND will always return a weight of 2
or more, then if the whole AND needs to return at least 6, the right
branch is told it needs to return at least 4.

Based on these minimum required and maximum possible weights, operator
decay can happen.

For example, an OR knows that if its left branch can contribute at most
a weight of 4 and its right branch at most 7, then if the minimum weight
required is 8, only documents matching both branches are now of interest
so it decays to an AND.  If the minimum weight is 6 it decays to an
AND\_MAYBE (A AND\_MAYBE B matches documents which match A, but B also
contributes to the weight - in most search engines query syntax, that's
expressed as `+A B`).  If the minimum weight needed is 12, no document
can be good enough.  Currently in this case the OR decays to AND and the
unattainability is then handled by the AND or at a higher level (either by
a parent PostList in the matcher's main loop which will terminate early
if the maxweight for the whole PostList tree is less than the smallest
weight in the protomset).

Operator decay is flagged using the same mechanism as autoprune, by
returning a pointer to the replacement PostList object from next or
skip\_to.

Possible decays:

-  OrPostList |->| AndPostList
-  OrPostList |->| AndMaybePostList
-  AndMaybePostList |->| AndPostList
-  XorPostList |->| AndNotPostList (not currently implemented)
