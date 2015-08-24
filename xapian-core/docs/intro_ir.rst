.. |->| unicode:: U+2192 .. right arrow
.. |ft| replace:: ``f``\ :sub:`t`
.. |qt| replace:: ``q``\ :sub:`t`
.. |Ld| replace:: ``L``\ :sub:`d`
.. |D0| replace:: ``D``\ :sub:`0`
.. |D1| replace:: ``D``\ :sub:`1`
.. |D2| replace:: ``D``\ :sub:`2`
.. |D3| replace:: ``D``\ :sub:`3`
.. |Di| replace:: ``D``\ :sub:`i`
.. |Dj| replace:: ``D``\ :sub:`j`
.. |DK| replace:: ``D``\ :sub:`K`
.. |t0| replace:: ``t``\ :sub:`0`
.. |t1| replace:: ``t``\ :sub:`1`
.. |t2| replace:: ``t``\ :sub:`2`
.. |t3| replace:: ``t``\ :sub:`3`
.. |ti| replace:: ``t``\ :sub:`i`
.. |tj| replace:: ``t``\ :sub:`j`
.. |tK| replace:: ``t``\ :sub:`K`
.. |tQ| replace:: ``t``\ :sub:`Q`
.. |k3| replace:: ``k``\ :sub:`3`
.. |Ri| replace:: ``R``\ :sub:`i`
.. |w1| replace:: ``w``\ :sub:`1`
.. |w2| replace:: ``w``\ :sub:`2`
.. |w3| replace:: ``w``\ :sub:`3`
.. |w9| replace:: ``w``\ :sub:`9`
.. |w38| replace:: ``w``\ :sub:`38`
.. |w97| replace:: ``w``\ :sub:`97`
.. |w221| replace:: ``w``\ :sub:`221`
.. |wm| replace:: ``w``\ :sub:`m`

Theoretical Background
======================

This document aims to provide some theoretical background to Xapian.

Documents and terms
-------------------

In Information Retrieval (IR), the items we are trying to retrieve are
called *documents*, and each document is described by a collection of
*terms*. These two words, \`document' and \`term', are now traditional
in the vocabulary of IR, and reflect its Library Science origins.
Usually a document is thought of as a piece of text, most likely in a
machine readable form, and a term as a word or phrase which helps to
describe the document, and which may indeed occur one or more times in
the document. So a document might be about dental care, and could be
described by corresponding terms \`tooth', \`teeth', \`toothbrush',
\`decay', \`cavity', \`plaque', \`diet' and so on.

More generally a document can be anything we want to retrieve, and a
term any feature that helps describe the documents. So the documents
could be a collection of fossils held in a big museum collection, and
the terms could be morphological characteristics of the fossils. Or the
documents could be tunes, and the terms could then be phrases of notes
that occur in the tunes.

If, in an IR system, a document, D, is described by a term, t, t is said
to *index* D, and we can write,

    ``t`` |->| ``D``

In fact an IR system consists of a set of documents, |D1|, |D2|, |D3| ...,
a set of terms |t1|, |t2|, |t3| ..., and set of relationships,

    |ti| |->| |Dj|

i.e. instances of terms indexing documents. A single instance of a
particular term indexing a particular document is called a *posting*.

For a document, D, there is a list of terms which index it. This is
called the *term list* of D.

For a term, t, there is a list of documents which it indexes. This is
called the *posting list* of t. (\`Document list' would be more
consistent, but sounds a little too vague for this very important
concept.)

At a simple level a computerised IR system puts the terms in an *index*
file. A term can be efficiently looked up and its posting list found. In
the posting list, each document is represented by a short identifier. To
over-simplify a little, a posting list can be thought of as a list of
numbers (document ids), and term list as a list of strings (the terms).
Some systems represent each term by a number internally, so the term
list is then also a list of numbers. Xapian doesn't - it uses the terms
themselves, and uses prefix compression to store them compactly.

The terms needn't be (and often aren't) just the words from the
document. Usually they are converted to lower case, and often a stemming
algorithm is applied, so a single term \`connect' might derive from a
number of words, \`connect', \`connects', \`connection', \`connected'
and so on. A single word might also give rise to more than one term, for
example you might index both stemmed and unstemmed forms of some or all
terms. Or a stemming algorithm could conceivably produce more than one
stem in some cases (this isn't the case for any of the stemming
algorithms Xapian currently supports, but consider the `Double
Metaphone <http://en.wikipedia.org/wiki/Double_Metaphone>`_ phonetic
algorithm which can produce two codes from a single input).

Xapian's context within IR
--------------------------

In the beginning IR was dominated by Boolean retrieval, described in the
next section. This could be called the antediluvian period, or
generation zero. The first generation of IR research dates from the
early sixties, and was dominated by model building, experimentation, and
heuristics. The big names were `Gerard
Salton <http://en.wikipedia.org/wiki/Gerard_Salton>`_ and `Karen Sparck
Jones <http://en.wikipedia.org/wiki/Karen_Sparck_Jones>`_. The second
period, which began in the mid-seventies, saw a big shift towards
mathematics, and a rise of the IR model based upon probability theory -
probabilistic IR. The big name here was, and continues to be, `Stephen
Robertson <http://www.soi.city.ac.uk/~ser/homepage.html>`_. More
recently `Keith van
Rijsbergen <http://en.wikipedia.org/wiki/C._J._van_Rijsbergen>`_ has led
a group that has developed underlying logical models of IR, but
interesting as this new work is, it has not as yet led to results that
offer improvements for the IR system builder.

Xapian was built as a system for efficiently implementing the
probabilistic IR model (though this doesn't mean it is limited to only
implementing this model - other models can be implemented providing they
can be expressed in a suitable way). Xapian tries to implement the
probabilistic model faithfully, though in some places it can be told to
use short-cuts for efficiency.

The model has two striking advantages:

#. It leads to systems that give good retrieval performance. As the
   model has developed over the last 25 years, this has proved so
   consistently true that one is led to suspect that the probability
   theory model is, in some sense, the \`correct' model for IR. The IR
   process would appear to function as the model suggests.
#. As new problems come up in IR, the probabilistic model can usually
   suggest a solution. This makes it a very practical mental tool for
   cutting through the jungle of possibilities when designing IR
   software.

In simple cases the model reduces to simple formulae in general use, so
don't be alarmed by the apparent complexity of the equations below. We
need them for a full understanding of the general case.

Boolean retrieval
-----------------

A Boolean construct of terms retrieves a corresponding set of documents.
So, if:

    |    |t1| indexes documents  1 2 3 5 8
    |    |t2| indexes documents  2 3 6

then

    |    |t1| ``AND`` |t2|      retrieves  2 3
    |    |t1| ``OR`` |t2|       retrieves  1 2 3 5 6 8
    |    |t1| ``AND_NOT`` |t2|  retrieves  1 5 8
    |    |t2| ``AND_NOT`` |t1|  retrieves  6

The posting list of a term is a set of documents. IR becomes a matter of
constructing other sets by doing unions, intersections and differences
on posting lists.

For example, in an IR system of works of literature, a Boolean query
::

        (lang:en OR lang:fr OR lang:de) AND (type:novel OR type:play) AND century:19

might be used to retrieve all English, French or German novels or plays
of the 19th century.

Boolean retrieval is often useful, but is rather inadequate on its own
as a general IR tool. Results aren't ordered by any measure of how
"good" they might be, and users require training to make effective use
of such a system. Despite this, purely boolean IR systems continue to
survive.

By default, Xapian uses probabilistic ranking to order retrieved
documents while allowing Boolean expressions of arbitrary complexity
(some boolean IR systems are restricted to queries in normal form) to
limit those documents retrieved, which provides the benefits of both
approaches. Pure Boolean retrieval is also supported (select the
`BoolWeight <apidoc/html/classXapian_1_1BoolWeight.html>`_ weighting
scheme using ``enquire.set_weighting_scheme(Xapian::BoolWeight());``).

Relevance and the idea of a query
---------------------------------

*Relevance* is a central concept to the probabilistic model. Whole
academic papers have been devoted to discussing the nature of relevance
but essentially a document is relevant if it was what the user really
wanted! Retrieval is rarely perfect, so among documents retrieved there
will be non-relevant ones; among those not retrieved, relevant ones.

Relevance is modelled as a black or white attribute. There are no
degrees of relevance, a document either is, or is not, relevant. In the
probabilistic model there is however a probability of relevance, and
documents of low probability of relevance in the model generally
correspond to documents that, in practice, one would describe as having
low relevance.

What the user actually wants has to be expressed in some form, and the
expression of the user's need is the query. In the probabilistic model
the query is, usually, a list of terms, but that is the end process of a
chain of events. The user has a need; this is expressed in ordinary
language; this is then turned into a written form that the user judges
will yield good results in an IR system, and the IR system then turns
this form into a set, *Q*, of terms for processing the query. Relevance
must be judged against the user's original need, not against a later
interpretation of what *Q*, the set of terms, ought to mean.

Below, a query is taken to be just a set of terms, but it is important
to realise that this is a simplification. Each link in the chain that
takes us from the *information need* ("what the user is looking for") to
the abstraction in *Q* is liable to error, and these errors compound to
affect IR performance. In fact the performance of IR systems as a whole
is much worse than most people generally imagine.

Evaluating IR performance
-------------------------

It is possible to set up a test to evaluate an IR system. Suppose *Q* is
a query, and out of the complete collection of documents in the IR
system, a set of documents *R* of size R are relevant to the query. So
if a document is in *R* it is relevant, and if not in *R* it is
non-relevant. Suppose the IR system is able to give us back K documents,
among which r are relevant. *Precision* and *recall* are defined as
being,

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td><tt>precision =&nbsp;</tt></td>
    <td>
    <tt><center>
    <u>r</u><br>K</center></tt>
    </td>
    <td><tt>,&nbsp;&nbsp;&nbsp;recall =&nbsp;</tt></td>
    <td>
    <tt><center>
    <u>r</u><br>R</center></tt>
    </td>
    </tr></table>
    </blockquote>


Precision is the density of relevant documents among those retrieved.
Recall is the proportion of relevant documents retrieved. In most IR
systems K is a parameter that can be varied, and what you find is that
when K is low you get high precision at the expense of low recall, and
when K is high you get high recall at the expense of low precision.

The ideal value of K will depend on the use of the system. For example,
if a user wants the answer to a simple question and the system contains
many documents which would answer it, a low value of K will be best to
give a small number of relevant results. But in a system indexing legal
cases, users will often wish to make sure no potentially relevant case
is missed even if that requires they check more non-relevant cases, so a
high value of K will be best.

Retrieval effectiveness is often shown as a graph of precision against
recall average over a number of queries, and plotted for different
values of K. Such curves typically have a shape similar to a hyperbola
(y=1/x).

A collection like this, consisting of a set of documents, a set of
queries, and for each query, a complete set of relevance assessments, is
called a *test collection*. With a test collection you can test out
different IR ideas, and see how well one performs against another. The
controversial part of establishing any test collection is the procedure
employed for determining the sets |Ri|, of relevance
assessments. Subjectivity of judgement comes in here, and people will
differ about whether a particular document is relevant to a particular
query. Even so, the averaging across queries reduces the errors that may
occasionally arise through faulty relevance judgements, and averaging
important tests across a number of test collections reduces the effects
caused by accidental features of individual collections, and the results
obtained by these tests in modern research are generally accepted as
trustworthy. Nowadays such research with test collections is organised
from `TREC <http://trec.nist.gov/>`_.

Probabilistic term weights
--------------------------

In this section we will try to present some of the thinking behind the
formulae. This is really to give a feel for where the probabilistic
model comes from. You may want to skim through this section if you're
not too interested.

Suppose we have an IR system with a total of N documents. And suppose
*Q* is a query in this IR system, made up of terms |t1|,
|t2| ... |tQ|. There is a set, *R*, of documents
relevant to the query.

In 1976, Stephen Robertson derived a formula which gives an ideal
numeric weight to a term t of Q. Just how this weight gets used we will
see below, but essentially a high weight means an important term and a
low weight means an unimportant term. The formula is,

.. raw:: html

   <blockquote>
   <table border=0><tr valign=center>
   <td><tt>w(t) = log&nbsp;</tt></td>
   <td>
   <font size="+2">(</font>
   </td>
   <td>
   <tt><center>
   <u>p (1 - q)</u><br>(1 - p) q</center></tt>
   </td>
   <td>
   <font size="+2">)</font>
   </td>
   </tr></table>
   </blockquote>

(The base of the logarithm doesn't matter, but we can suppose it is e.)
p is the probability that t indexes a relevant document, and q the
probability that t indexes a non-relevant document. And of course, 1 - p
is the probability that t does not index a relevant document, and 1 - q
the probability that t does not index a non-relevant document. More
mathematically,

        p = P(t |->| D | D in R)
        q = P(t |->| D | D not in R)

        1 - p = P(t not |->| D | D in R)
        1 - q = P(t not |->| D | D not in R)

Suppose that t indexes n of the N documents in the IR system. As before,
we suppose also that there are R documents in *R*, and that there are r
documents in *R* which are indexed by t.

p is easily estimated by r/R, the ratio of the number of relevant
documents indexed by t to the total number of relevant documents.

The total number of non-relevant documents is N - R, and the number of
those indexed by t is n - r, so we can estimate q as (n - r)/(N - R).
This gives us the estimates,

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td><tt>&nbsp;&nbsp;&nbsp;&nbsp;p =&nbsp;</tt></td>
    <td>
    <tt><center>
    <u>r</u><br>R</center></tt>
    </td>
    <td><tt>,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;1 - q =&nbsp;</tt></td>
    <td>
    <tt><center>
    <u>N - R - n + r</u><br>N - R</center></tt>
    </td>
    </tr></table>
    <table border=0><tr valign=center>
    <td><tt>1 - p =&nbsp;</tt></td>
    <td>
    <tt><center>
    <u>R - r</u><br>R</center></tt>
    </td>
    <td><tt>,&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;q =&nbsp;</tt></td>
    <td>
    <tt><center>
    <u>n - r</u><br>N - R</center></tt>
    </td>
    </tr></table>
    </blockquote>

and so substituting in the formula for w(t) we get the estimate,

.. raw:: html

   <blockquote>
   <table border=0><tr valign=center>
   <td>
   <tt>w(t) = log&nbsp;</tt>
   </td>
   <td>
   <font size="+2">(</font>
   </td>
   <td>
   <tt><center>
   <u>r (N - R - n + r)</u><br>(R - r)(n - r)</center></tt>
   </td>
   <td>
   <font size="+2">)</font>
   </td>
   </tr></table>
   </blockquote>

Unfortunately, this formula is subject to violent behaviour when, say, n
= r (infinity) or r = 0 (minus infinity), and so Robertson suggests the
modified form

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td>
    <tt>w(t) = log&nbsp;</tt>
    </td>
    <td>
    <font size="+2">(</font>
    </td>
    <td>
    <tt><center>
    <u>(r + &frac12;) (N - R - n + r + &frac12;)</u><br>(R - r + &frac12;) (n - r + &frac12;)</center></tt>
    </td>
    <td>
    <font size="+2">)</font>
    </td>
    </tr></table>
    </blockquote>

with the reassurance that this has "some theoretical justification".
This is the form of the term weighting formula used in Xapian's
BM25Weight.

Note that n is dependent on the term, t, and R on the query, *Q*, while
r depends both on t and *Q*. N is constant, at least until the IR system
changes.

At first sight this formula may appear to be quite useless. After all,
*R* is what we are trying to find. We can't evaluate w(t) until we have
*R*, and if we have *R* the retrieval process is over, and term weights
are no longer of any interest to us.

But the point is we can estimate p and q from a subset of *R*. As soon
as some records are found relevant by the user they can be used as a
working set for *R* from which the weights w(t) can be derived, and
these new weights can be used to improve the processing of the query.

In fact in the Xapian software *R* tends to mean not the complete set of
relevant documents, which indeed can rarely be discovered, but a small
set of documents which have been judged as relevant.

Suppose we have no documents marked as relevant. Then R = r = 0, and
w(t) becomes,

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td><tt>log&nbsp;</tt></td>
    <td>
    <font size="+2">(</font>
    </td>
    <td>
    <tt><center>
    <u>N - n + &frac12;</u><br>n + &frac12;</center></tt>
    </td>
    <td>
    <font size="+2">)</font>
    </td>
    </tr></table>
    </blockquote>

This is approximately log((N - n)/n). Or log(N/n), since n is usually
small compared with N. This is called inverse logarithmic weighting, and
has been used in IR for many decades, quite independently of the
probabilistic theory which underpins it. Weights of this form are in
fact the starting point in Xapian when no relevance information is
present.

The number n incidentally is often called the *frequency* of a term. We
prefer the phrase *term frequency*, to better distinguish it from wdf
and wqf introduced below.

In extreme cases w(t) can be negative. In Xapian, negative values are
disallowed, and simply replaced by a small positive value.

wdp, wdf, ndl and wqf
---------------------

Before we see how the weights are used there are a few more ideas to
introduce.

As mentioned before, a term t is said to index a document D, or t |->| D.
We have emphasised that D may not be a piece of text in machine-readable
form, and that, even when it is, t may not actually occur in the text of
D. Nevertheless, it will often be the case that D is made up of a list
of words,

            ``D =`` |w1|, |w2|, |w3| ... |wm|

and that many, if not all, of the terms which index D derive from these
words (for example, the terms are often lower-cased and stemmed forms of
these words).

If a term derives from words |w9|, |w38|, |w97| and |w221| in the indexing
process, we can say that the term \`occurs' in D at positions 9, 38, 97 and
221, and so for each term a document may have a vector of positional
information. These are the *within-document positions* of t, or the *wdp*
information of t.

The *within-document frequency*, or *wdf*, of a term t in D is the
number of times it is pulled out of D in the indexing process. Usually
this is the size of the wdp vector, but in Xapian it can exceed it,
since we can apply extra wdf to some parts of the document text. For
example, often this is done for the document title and abstract to
attach extra importance to their contents compared to the rest of the
document text.

There are various ways in which we might measure the length of a
document, but the easiest is to suppose it is made up of m words,
|w1| to |wm|, and to define its length as m.

The *normalised document length*, or *ndl*, is then m divided by the
average length of the documents in the IR system. So the average length
document has ndl equal to 1, short documents are less than 1, long
documents greater than 1. We have found that very small ndl values
create problems, so Xapian actually allows for a non-zero minimum value
for the ndl.

In the probabilistic model the query, *Q*, is itself very much like
another document. Frequently indeed *Q* will be created from a document,
either one already in the IR system, or by an indexing process very
similar to the one used to add documents into the whole IR system. This
corresponds to a user saying "give me other documents like this one".
One can therefore attach a similar meaning to within-query position
information, within-query frequency, and normalised query length, or
wqp, wqf and nql. Xapian does not currently use the concept of wqp.

Using the weights. The *MSet*
-----------------------------

Now to pull everything together. From the probabilistic term weights we
can assign a weight to any document, d, as follows,

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td><tt>W(d) =&nbsp;</tt></td>
    <td>
    <tt><center>
    <font size="+4">&Sigma;</font><br><small>t &rarr; d, t in <i>Q</i></small></tt>
    </td>
    <td><tt><center>
    <u>(k + 1) f</u><sub>t</sub><br>k.L<sub>d</sub> + f<sub>t</sub>
    </center></tt></td>
    <td><tt>&nbsp;w(t)</tt></td>
    </tr></table>
    </blockquote>

The sum extends over the terms of *Q* which index d. |ft| is
the wdf of t in d, |Ld| is the ndl of d, and k is some suitably
chosen constant.

The factor ``k+1`` is actually redundant, but helps with the interpretation
of the equation. In Xapian, this weighting scheme is implemented by the
`Xapian::TradWeight class <apidoc/html/classXapian_1_1TradWeight.html>`_
and the factor ``(k+1)`` is ignored.

If ``k`` is set to zero the factor before ``w(t)`` is 1, and the wdfs are
ignored. As ``k`` tends to infinity, the factor becomes
|ft| ``/`` |Ld|, and the wdfs take on their greatest
importance. Intermediate values scale the wdf contribution between these
extremes. The best ``k`` actually depends on the characteristics of the IR
system as a whole, and unfortunately no rule can be given for choosing
it. By default, Xapian sets ``k`` to 1 which should give reasonable results
for most systems. ``W(d)`` is merely tweaked a bit by the wdf values, and
users observe a simple pattern of retrieval. It is possible to tune ``k`` to
provide optimal results for a specific system.

Any ``d`` in the IR system has a value ``W(d)``, but, if no term of the query
indexes ``d``, ``W(d)`` will be zero. In practice only documents for which
``W(d)>0`` will be of interest, and these are the documents indexed by at least
one term of *Q*. If we now take these documents and arrange them by
decreasing ``W(d)`` value, we get a ranked list called the *match set*, or
*MSet*, of document and weight pairs:

    | ``item 0:``   |D0|, W(|D0|)
    | ``item 1:``   |D1|, W(|D1|)
    | ``item 2:``   |D2|, W(|D2|)
    | ...
    | ``item K:``   |DK|, W(|DK|)

where W(|Dj|) >= W(|Di|) if j > i.

And according to the probabilistic model, the documents |D0|, |D1|, |D2| ...
are ranked by decreasing order of probability of relevance. So |D0| has highest
probability of being relevant, then |D1| and so on.

Xapian creates the MSet from the posting lists of the terms of the
query. This is the central operation of any IR system, and will be
familiar to anyone who has used one of the Internet's major search
engines, where the query is what you type in the query box, and the
resulting hit list corresponds to the top few items of the MSet.

The cutoff point, K, is chosen when the MSet is created. The candidates
for inclusion in the MSet are all documents indexed by at least one term
of *Q*, and their number will usually exceed the choice of K (K is
typically set to be 1000 or less). So the MSet is actually the best K
documents found in the match process.

A modification of this weighting scheme can be employed that takes into
account the query itself:

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td><tt>W(d) =&nbsp;</tt></td>
    <td>
    <tt><center>
    <font size="+4">&Sigma;</font><br><small>t &rarr; d, t in <i>Q</i></small></center></tt>
    </td>
    <td><tt><center>
    <u>(k<sub>3</sub> + 1) q</u><sub>t</sub><br>k<sub>3</sub>L' + q<sub>t</sub>
    </center></tt></td>
    <td><tt>&nbsp;</tt></td>
    <td><tt><center>
    <u>(k + 1) f</u><sub>t</sub><br>kL<sub>d</sub> + f<sub>t</sub>
    </center></tt></td>
    <td><tt>&nbsp;w(t)</tt></td>
    </tr></table>
    </blockquote>

where |qt| is the wqf of t in *Q*, ``L'`` is the nql, or normalised
query length, and |k3| is a further constant. In computing W(d)
across the document space, this extra factor may be viewed as just a
modification to the basic term weights, ``w(t)``. Like ``k`` and |k3|,
we will need to make an inspired guess for ``L'``. In fact the choices for
|k3| and ``L'`` will depend on the broader context of the use of
this formula, and more advice will be given as occasion arises.

Xapian's default weighting scheme is a generalised form of this
weighting scheme modification, known as `BM25 <bm25.html>`_. In BM25, ``L'``
is always set to 1.

Using the weights: the *ESet*
-----------------------------

But as well as ranking documents, Xapian can rank terms, and this is
most important. The higher up the ranking the term is, the more likely
it is to act as a good differentiator between relevant and non-relevant
documents. It is therefore a candidate for adding back into the query.
Terms from this list can therefore be used to expand the size of the
query, after which the query can be re-run to get a better MSet. Because
this list of terms is mainly used for query expansion, it is called the
*expand set* or *ESet*.

The term expansion weighting formula is as follows,
::

        W(t) = r w(t)

in other words we multiply the term weight by the number of relevant
documents that have been indexed by the term.

The ESet then has this form,

    | ``item 0:``   |t0|, W(|t0|)
    | ``item 1:``   |t1|, W(|t1|)
    | ``item 2:``   |t2|, W(|t2|)
    | ...
    | ``item K:``   |tK|, W(|tK|)

where W(|tj|) >= W(|ti|) if j > i.

Since the main function of the ESet is to find new terms to be added to
*Q*, we usually omit from it terms already in *Q*.

The ``W(t)`` weight is applicable to any term in the IR system, but has a
value zero when t does not index a relevant document. The ESet is
therefore confined to be a ranking of the best K terms which index
relevant documents.

This simple form of ``W(t)`` is traditional in the probabilistic model, but
seems less than optimal because it does not take into account wdf
information. One can if fact try to generalise it to:

.. raw:: html

    <blockquote>
    <table border=0><tr valign=center>
    <td><tt>W(t) =&nbsp;</tt></td>
    <td>
    <tt><center>
    <font size="+4">&Sigma;</font><br><small>t &rarr; d, d in <i>R</i></small></tt>
    </td>
    <td><tt><center>
    <u>(k + 1) f</u><sub>t</sub><br>kL + f<sub>t</sub>
    </center></tt></td>
    <td><tt>&nbsp;w(t)</tt></td>
    </tr></table>
    </blockquote>

``k`` is again a constant, but it does not need to have the same value as
the ``k`` used in the probabilistic term weights above. In Xapian, ``k``
defaults to 1.0 for ESet generation.

This reduces to ``W(t) = r w(t)`` when ``k=0``. Certainly this form can be
recommended in the very common case where ``r=1``, that is, we have a
single document marked relevant.

The progress of a query
-----------------------

Below we describe the general case of the IR model supported, including
use of a relevance set (`RSet <glossary.html#rset>`_), query expansion,
improved term weights and reranking. You don't have to use any of these
for Xapian to be useful, but they are available should you need them.

The user enters a query. This is parsed into a form the IR system
understands, and run by the IR system, which returns two lists, a list
of captions, derived from the MSet, and a list of terms, from the ESet.
If the RSet is empty, the first few documents of the MSet can be used as
a stand-in - after all, they have a good chance of being relevant! You
can read a document by clicking on the caption. (We assume the usual
screen/mouse environment.) But you can also mark a document as relevant
(change *R*) or cause a term to be added from the ESet to the query
(change *Q*). As soon as any change is made to the query environment the
query can be rerun, although you might have a front-end where nothing
happens until you click on some "Run Query" button.

In any case rerunning the query leads to a new MSet and ESet, and so to
a new display. The IR process is then an iterative one. You can delete
terms from the query or add them in; mark or unmark documents as being
relevant. Eventually you converge on the answer to the query, or at
least, the best answer the IR system can give you.

Further Reading
---------------

If you want to find out more, then `"Simple, proven approaches to text
retrieval" <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.53.8337>`_
is a worthwhile read. It's a good introduction to Probabilistic
Information retrieval, which is basically what Xapian provides.

There are also several good books on the subject of Information
retrieval.

-  "*Information Retrieval*" by C. J. van Rijsbergen is well worth
   reading. It's out of print, but is available for free `from the
   author's website <http://www.dcs.gla.ac.uk/Keith/Preface.html>`_ (in
   HTML or PDF).
-  "*Readings in Information Retrieval*" (published by Morgan Kaufmann,
   edited by Karen Sparck Jones and Peter Willett) is a collection of
   published papers covering many aspects of the subject.
-  "*Managing Gigabytes*" (also published by Morgan Kaufmann, written by
   Ian H. Witten, Alistair Moffat and Timothy C. Bell) describes
   information retrieval and compression techniques.
-  "*Modern Information Retrieval*" (published by Addison Wesley,
   written by Ricardo Baeza-Yates and Berthier Ribeiro-Neto) gives a
   good overview of the field. It was published more recently than the
   books above, and so covers some more recent developments.
-  "*Introduction to Information Retrieval*" (published by Cambridge
   University Press, written by Christopher D. Manning, Prabhakar
   Raghavan and Hinrich Schütze) looks to be a good introductory work
   (we've not read it in detail yet). As well as the print version,
   there's an online version on `the book's companion
   website <http://www-csli.stanford.edu/~hinrich/information-retrieval-book.html>`_.

