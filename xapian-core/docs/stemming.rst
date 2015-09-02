.. |<->| unicode:: U+2194 .. left right arrow

Stemming Algorithms
===================

Xapian uses the `Snowball Stemming
Algorithms <http://snowballstem.org/>`_. At present, these support
Danish, Dutch, English, Finnish, French, German, Hungarian, Italian,
Norwegian, Portuguese, Romanian, Russian, Spanish, Swedish,
and Turkish.

There are also implementations of Lovins' English stemmer, Porter's
original English stemmer, the Kraaij-Pohlmann Dutch stemmer, and a
variation of the German stemmer which normalises umlauts.

We'd like to add stemmers for more languages - see the Snowball site for
information on how to contribute.

What is a stemming algorithm?
-----------------------------

A stemming algorithm is a process of linguistic normalisation, in which
the variant forms of a word are reduced to a common form, for example,
::

        connection
        connections
        connective          --->   connect
        connected
        connecting

It is important to appreciate that we use stemming with the intention of
improving the performance of IR systems. It is not an exercise in
etymology or grammar. In fact from an etymological or grammatical
viewpoint, a stemming algorithm is liable to make many mistakes. In
addition, stemming algorithms - at least the ones presented here - are
applicable to the written, not the spoken, form of the language.

For some of the world's languages, Chinese for example, the concept of
stemming is not applicable, but it is certainly meaningful for the many
languages of the Indo-European group. In these languages words tend to
be constant at the front, and to vary at the end::

                       -ion
                       -ions
                connect-ive
                       -ed
                       -ing

The variable part is the \`ending', or \`suffix'. Taking these endings
off is called \`suffix stripping' or \`stemming', and the residual part
is called the stem.

Endings
-------

Another way of looking at endings and suffixes is to think of the suffix
as being made up of a number of endings. For example, the French word
::

                confirmatives

can be thought of as \`confirm' with a chain of endings,
::

                -atif (adjectival ending - morphological)
        plus    -e    (feminine ending - grammatical)
        plus    -s    (plural ending - grammatical)

-atif can also be thought of as -ate plus -if. Note that the addition of
endings can cause respellings, so -e changes preceding \`f' to \`v'.

Endings fall into two classes, grammatical and morphological. The
addition of -s in English to make a plural is an example of a
grammatical ending. The word remains of the same type. There is usually
only one dictionary entry for a word with all its various grammatical
endings. Morphological endings create new types of word. In English -ise
or -ize makes verbs from nouns (\`demon', \`demonise'), -ly makes
adverbs from adjectives (\`foolish', \`foolishly'), and so on. Usually
there are separate dictionary endings for these creations.

Language knowledge
------------------

It is much easier to write a stemming algorithm for a language when you
are familiar with it. If you are not, you will probably need to work
with someone who is, and who can also explain details of grammar to you.
Best is a professional teacher or translator. You certainly don't need
to have a world authority on the grammar of the language. In fact too
much expertise can get in the way when it comes to the very practical
matter of writing the stemming algorithm.

Vocabularies
------------

Each stemmer is issued with a vocabulary in data/voc.txt, and its
stemmed form in data/voc.st. You can use these for testing and
evaluation purposes.

Raw materials
-------------

A conventional grammar of a language will list all the grammatical
endings, and will often summarise most of the morphological endings. A
grammar, plus a dictionary, are therefore basic references in the
development of a stemming algorithm, although you can dispense with them
if you have an excellent knowledge of the language. What you cannot
dispense with is a vocabulary to try the algorithm out on as it is being
developed. Assemble about 2 megabytes of text. A mix of sources is best,
and literary prose (conventional novels) usually gives an ideal mix of
tenses, cases, persons, genders etc. Obviously the texts should be in
some sense 'contemporary', but it is an error to exclude anything
slightly old. The algorithm itself may well get applied to older texts
once it has been written. For English, the works of Shakespeare in the
customary modern spelling make a good test vocabulary.

From the source text derive a control vocabulary of words in sorted
order. Sample vocabularies in this style are part of our Open Source
release. If you make a small change to the stemming algorithm you should
have a procedure that presents the change as a three column table:
column one is the control vocabulary, column 2 the stemmed equivalent,
and column 3 the stemmed equivalent after the change has been made to
the algorithm. The effects of the change can be evaluated by looking at
the differences between columns two and three.

The first job is to come up with a list of endings. This can be done by
referring to the grammar, the dictionary, and also by browsing through
the control vocabulary.

Rules for removing endings
--------------------------

If a word has an ending, E, when should E be removed? Various criteria
come into play here. One is the knowledge we have about the word from
other endings that might have been removed. If a word ends with a
grammatical verb ending, and that has been removed, then we have a verb
form, and the only further endings to consider are morphological endings
that create verbs from other word types. At this level the system of
endings gives rise to a small state table, which can be followed in
devising the algorithm. In Latin derived languages, there is a state
table of morphological endings that roughly looks like this::

       -IC (adj) -+->  -ATION (noun)
                  +->  -ITY (noun)
                  +->  -MENT (adv)
                  \->  -AT (verb)     -+->   -IV (adj)    -+->   -ITY (noun)
                                       |                   \->   -MENT (adv)
                                       \->   -OR (noun)

     -ABLE (adj) -+->  -ITY (noun)
                  \->  -MENT (adv)

      -OUS (adj) --->  -MENT (adv)

The ending forms take different values in different languages. In
French, -OR becomes \`-eur' (m.) or \`-rice' (f.), -AT disappears into
the infinitive form of a verb. In English, -MENT becomes \`-ly', and
then one can recognise,
::

       -IC-ATION   fortification
       -IC-ITY     electricity
       -IC-MENT    fantastically
       -AT-IV      contemplative
       -AT-OR      conspirator
       -IV-ITY     relativity
       -IV-MENT    instinctively
       -ABLE-ITY   incapability
       -ABLE-MENT  charitably
       -OUS-MENT   famously

Trios, -IC-AT-IV etc., also occur, but sequences of length four,
-IC-AT-IV-ITY and -IC-AT-IV-MENT, are absent (or occur very rarely).

Sometimes the validity of an ending depends on the immediately preceding
group of letters. In Italian, for example, certain pronouns and pronoun
groups attach to present participle and infinitive forms of verbs, for
example,
::

    scrivendole = scrivendo (writing) + le (to her)
    mandarglielo = mandare (to give) + glielo (it to him)

If E is the ending, the possible forms are -andoE, -endoE, -arE, -erE,
-irE, so E is removed in the context -Xndo or Yr, where X is a or e, and
Y is a or e or i. See the ``attached_pronoun`` procedure in the Italian
stemmer.

The most useful criterion for removing an ending, however, is to base
the decision on the syllable length of the stem that will remain. This
idea was first used in the English stemming algorithm, and has been
found to be applicable in the other stemming algorithms too. If C stands
for a sequence of consonants, and V for a sequence of vowels, any word
can be analysed as,
::

            [C] V C ... V C [V]

where [..] indicates arbitrary presence, and V C ... V C means V C
repeated zero or more times. We can find successive positions 0, 1, 2
... in a word corresponding to each vowel-consonant stretch V C,
::

            t h u n d e r s t r i c k e n
               0     1         2     3   4

The closer E is to the beginning of the word, the more unwilling we
should be remove it. So we might have a rule to remove E if at is after
position 2, and so on.

Developing the algorithm
------------------------

Build the algorithm up bit by bit, trying out a small number of ending
removals at a time. For each new ending plus rule added, decide whether,
on average, the stemming process is improved or degraded. If it is
degraded the rule is unhelpful and can be discarded.

This sounds like common sense, but it is actually very easy to fall into
the trap of endlessly elaborating the rules without looking at their
true effect. What you find eventually is that you can be improving
performance in one area of the vocabulary, while causing a similar
degradation of performance in another area. When this happens
consistently it is time to call a halt to development and to regard the
stemming algorithm as finished.

It is important to realise that the stemming process cannot be made
perfect. For example, in French, the simple verb endings -ons and -ent
of the present tense occur repeatedly in other contexts. -ons is the
plural form of all nouns ending -on, and -ent is also a common noun
ending. On balance it is best not to remove these endings. In practice
this affects -ent verb endings more than -ons verb endings, since -ent
endings are commoner. The result is that verbs stem not to a single
form, but to a much smaller number of forms (three), among which the
form given by the true stem of the verb is by far the commonest.

If we define errors A and B by,

- error A: removing an ending when it is not an ending
- error B: not removing an ending when it is an ending

Then removing -ent leads to error A; not removing -ent leads to error B.
We must adopt the rule that minimises the number of errors - not the
rule that appears to be the most elegant.

Irregular forms
---------------

Linguistic irregularities slip through the net of a stemming algorithm.
The English stemmer stems \`cows' to \`cow', but does not stem \`oxen'
to \`ox'. In reality this matters much less than one might suppose. In
English, the irregular plurals tend to be of things that were common in
Anglo-Saxon England: oxen, sheep, mice, dice - and lice. Men, women and
children are of course common today, but the very commonness of these
words makes them of less importance in IR systems. Similar remarks may
be said about irregular verbs in English, the total number of which is
around 150. Here, the fact that verbs are used perhaps rather less than
nouns and adjectives in IR queries helps account for the unimportance of
verb irregularities in IR performance. There are in English more
significant irregularities in morphological changes such as \`receive'
to \`reception', \`decide' to \`decision' etc., which correspond,
ultimately, to irregularities in the Latin verbs from which these words
derive. But again working IR systems are rarely upset by lack of
resolution of these forms.

An irregularity of English which does cause a problem is the word
\`news'. It is now a singular noun, and is never regarded as the plural
of \`new'. This, and a few more howlers, are placed in a table,
``irregular_forms``, in the English stemming algorithm. Similar tables
are provided in the other stemming algorithms, with some provisional
entries. The non-English stemming algorithms have not been used enough
for a significant list of irregular forms to emerge, but as they do,
they can be placed in the ``irregular_forms`` table.

Using stemming in IR
--------------------

In earlier implementations of IR systems, the words of a text were
usually stemmed as part of the indexing process, and the stemmed forms
only held in the main IR index. The words of each incoming query would
then be stemmed similarly. When the index terms were seen by the user,
for example during query expansion, they would be seen in their stemmed
form. It was important therefore that the stemmed form of a word should
not be too unfamiliar in appearance. A user will be comfortable with
seeing \`apprehend', which stands for 'apprehending', \`apprehended' as
well as \`apprehend'. More problematical is \`apprehens', standing for
\`apprehension', \`apprehensive' etc., but even so, a trained user would
not have a problem with this. In fact all the Xapian stemming algorithms
are built on the assumption that it leave stemmed forms which it would
not be embarrassing to show to real users, and we suggest that new
stemming algorithms are designed with this criterion in mind.

A superior approach is to keep each word, *W*, and its stemmed form,
*s(W)*, as a two-way relation in the IR system. *W* is held in the index
with its own posting list. *s(W)* could have its separate posting list,
but this would be derivable from the class of words that stem to *s(W)*.
The important thing is to have the *W* |<->| *s(W)* relation. From *W* we
can derive *s(W)*, the stemmed form. From a stemmed form *s(W)* we can
derive *W* plus the other words in the IR system which stem to *s(W)*.
Any word can then be searched on either stemmed or unstemmed. If the
stemmed form of a word needs to be shown to the user, it can be
represented by the commonest among the words which stem to that form.

Stopwords
---------

It has been traditional in setting up IR systems to discard the very
commonest words of a language - the stopwords - during indexing. A more
modern approach is to index everything, which greatly assists searching
for phrases for example. Stopwords can then still be eliminated from the
query as an optional style of retrieval. In either case, a list of
stopwords for a language is useful.

Getting a list of stopwords can be done by sorting a vocabulary of a
text corpus for a language by frequency, and going down the list picking
off words to be discarded.

The stopword list connects in various ways with the stemming algorithm:

The stemming algorithm can itself be used to detect and remove
stopwords. One would add into the ``irregular_forms`` table something
like this,
::

       "", /* null string */

       "am/is/are/be/being/been/"    /* BE */
       "have/has/having/had/"        /* HAD */
       "do/does/doing/did/"          /* DID */
       ...                           /* multi-line string */

so that the words \`am', \`is' etc. map to the null string (or some
other easily recognised value).

Alternatively, stopwords could be removed before the stemming algorithm
is applied, or after the stemming algorithm is applied. In this latter
case, the words to be removed must themselves have gone through the
stemmer, and the number of distinct forms will be greatly reduced as a
result. In Italian for example, the four forms
::

        questa     queste    questi    questo

(meaning \`that') all stem to
::

        quest

.. FIXME: Nice idea, but currently these lists are fictitious:
    In the xapian-data directory in the git repository, each language
    represented in the stemming section has, in addition to a large test
    vocabulary, a useful stopword list in both source and stemmed form. The
    source form, in the file ``stopsource``, is carefully annotated, and the
    derived file, ``stopwords``, contains an equivalent list of sorted,
    stemmed, stopwords.
