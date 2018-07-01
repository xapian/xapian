.. |->| unicode:: U+2192 .. right arrow

Matcher Design Notes
====================

This document is incomplete at present. It lacks explanation of the
min-heap used to keep the best N M-set items (Managing Gigabytes
describes this technique well), the check() method isn't discussed, and
probably some other things.

The PostList Tree
-----------------

The QueryOptimiser class builds a tree structure of PostList objects
from the query. At the leaf level, a PostList object is created for each
term, and for other leaf-level subqueries, like PostingSource objects
and value ranges. Then pairs or groups of PostLists are combined using
2-way or n-way branching tree elements for AND, OR, etc - these are
virtual PostLists whose class names reflect the operation
(MultiAndPostList, OrPostList, etc). See below for a full list.

OR
~~

For a group of OR operations, each OrPostList has two children, job).
The OR tree is built up in a similar way to how an optimal huffman code
is constructed, so the sub-PostLists with the fewest entries are
furthest down the tree, and those with most nearest the top (this is
more efficient than an n-ary tree in terms of the number of comparisons
which need to be performed, ignoring various optimisations which the
matcher can perform - it may actually be the case that a MultiOrPostList
could do a better job in practice though).

OR is coded for maximum efficiency when the right branch has fewer
postings in than the left branch.

When an OR gets "at end", it autoprunes, replacing itself with the
branch that still has postings - see below for full details.

AND
~~~

For a multi-way AND operation, we have MultiAndPostList, which tries the
sub-postlists in order from least frequent to most frequent (two-way AND
is handled the same way). This will generally minimise the number of
posting list entries we read and maximises the size of each skip\_to.

When one of a sub-trees of AND operations runs out, the sub-query will
signal "at end", and this causes the AND to signal "at end" too.

The OP\_FILTER query operator is actually treated as AND in the postlist
tree - the boolean-ness is pushed down to the leaf query, where it is
handled by the Weight object.

Other operations
~~~~~~~~~~~~~~~~

The other operations also handle "at end" either like OR or AND (for
asymmetric operations like AND\_MAYBE, which happens may depend which
branch has run out).

running the match
-----------------

Once the tree is built, the matcher repeatedly asks the root of the tree
for the next matching document and compares it to those in the
proto-mset it maintains. Once the proto-mset is of the desired final
size, the candidate needs to score more highly that the lowest scoring
document in the proto-mset (either by weight, or in sort order if
sorting is used) to be interesting. If it is, the lowest scoring
document is removed (which is easy as we store the proto-mset as a min
heap) and the candidate is added.

When the matcher itself gets "at end" from the postlist tree, the match
process ends.

The matcher also passes the lowest weight currently needed make the
proto-mset into the tree, and each node may adjust this weight and pass
it on to its subtrees. Each PostList can report a minimum weight it
could contribute - so if the left branch of an AND will always return a
weight of 2 or more, then if the whole AND needs to return at least 6,
the right branch is told it needs to return at least 4.

For example, an OR knows that if its left branch can contribute at most
a weight of 4 and its right branch at most 7, then if the minimum weight
is 8, only documents matching both branches are now of interest so it
mutates into an AND. If the minimum weight is 6 it changes into an
AND\_MAYBE (A AND\_MAYBE B matches documents which which match A, but B
contributes to the weight - in most search engines query syntax, that's
expressed as `+A B`). See the "Operator Decay" section below for full
details of these mutations. If the minimum weight needed is 12, no
document is good enough, and the OR returns "end of list".

Phrase and near matching
------------------------

The way phrase and near matching works is to perform an AND query for
all the terms, with a filter node in front which only returns documents
whose positional information fulfils the phrase requirements.

Because checking the positional information can be quite costly compared
to matching postlist trees, we hoist the position check higher up the
tree in cases when the phrase operation is below an AND. So A AND (B
NEAR C) will actually filter the results of (A AND B AND C) through a
check for B NEAR C, which means we never need to check positions for
documents which don't match A.

virtual postlist types
----------------------

There are several types of virtual PostList. Each type can be used in a
weighted or unweighted (boolean) context - the only difference is whether the
weights are used or not. The types are:

-  OrPostList: returns documents which match either branch
-  MultiAndPostList: returns documents which match all branches
-  MultiXorPostList: returns documents which match an odd number of
   branches
-  AndNotPostList: returns documents which match the left branch, but
   not the right (the weights of documents from the right branch are
   ignored).  "X ANDNOT Y" implements what some search engines offer
   as "X -Y" in their query syntax.
-  AndMaybePostList: returns documents which match the left branch with
   weights added on from the right branch where that also matches (so
   an unweighted AndMaybePostList isn't very useful).  "X ANDMAYBE Y"
   implements what some search engines offer as "+X Y" in their
   query syntax.
-  FIXME: this list is no longer complete...

There are two main optimisations which the best match performs:
autoprune and operator decay.

autoprune
---------

For example, if a branch in the match tree is "A OR B", when A runs out
then "A OR B" is replaced by "B". Similar reductions occur for XOR,
ANDNOT, and ANDMAYBE (if the right branch runs out). Other operators
(AND, FILTER, and ANDMAYBE (when the left branch runs out) simply return
"at\_end" and this is dealt with somewhere further up the tree as
appropriate.

An autoprune is indicated by the next or skip\_to method returning a
pointer to the PostList object to replace the postlist being read with.

operator decay
--------------

The matcher tracks the minimum weight needed for a document to make it
into the m-set (this decreases monotonically as the m-set forms). This
can be used to replace on boolean operator with a stricter one. E.g.
consider A OR B - when maxweight(A) < minweight and maxweight(B) <
minweight then only documents matching both A and B can make it into the
m-set so we can replace the OR with an AND. Operator decay is flagged
using the same mechanism as autoprune, by returning the replacement
operator from next or skip\_to.

Possible decays:

-  OR |->| AND
-  OR |->| ANDMAYBE
-  ANDMAYBE |->| AND
-  XOR |->| ANDNOT

A related optimisation is that the Match object may terminate early if
maxweight for the whole tree is less than the smallest weight in the
mset.
