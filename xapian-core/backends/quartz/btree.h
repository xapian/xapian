/* btree.h: Btree implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_BTREE_H
#define OM_HGUARD_BTREE_H

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte;
typedef long int4;
typedef unsigned long uint4;

struct Cursor {

    byte * p;         /* pointer to a block */
    int c;            /* offset in the block's directory */
    int4 n;           /* block number */
    int rewrite;      /* true if the block is not the same as on disk, and so needs rewriting */
    byte * split_p;   /* pointer to a block split off from main block */
    int4 split_n;     /* - and its block number */

};

/* n is kept in tandem with p. The unassigned state is when member p == 0 and n == -1.
   Similarly split.p == 0 corresponds to split.n == -1. Settings to -1 are not strictly
   neccessary in the code below, so the lines

        C[j].n = -1;
        C[j].split_n = -1;

   might sometimes be omitted, but they help keep the intention clear.
*/

#define BTREE_CURSOR_LEVELS 10
    /* allow for this many levels in the B-tree. Overflow practically impossible */

struct Btree {

    /* 'public' information */

    char overwritten;     /* set to true if a parallel overwrite is detected */
    uint4 revision_number;/* revision number of the opened B-tree */
    uint4 other_revision_number;
                          /* - revision number of the other base */
    char both_bases;      /* set to true if baseA and baseB both exist. The old base
                             is deleted as soon as a write to the Btree takes place */
    int4 item_count;      /* keeps a count of the number of items in the B-tree */
    int max_key_len;      /* the largest possible value of a key_len */

    /* 'semi-public': the user might be allowed to read this */

    int block_size;       /* block size of the B tree in bytes */
    int base_letter;      /* the value 'A' or 'B' of the current base */
    int4 last_block;      /* the last used block of B->bit_map0 */

    /* 'private' information */

    int handle;           /* corresponding file handle */
    int level;            /* number of levels, counting from 0 */
    int4 root;            /* the root block of the B-tree */
    byte * kt;            /* buffer of size B->block_size for making up key-tag items */
    byte * buffer;        /* buffer of size block_size for reforming blocks */
    uint4 next_revision;  /* 1 + revision number of the opened B-tree */
    int bit_map_size;     /* size of the bit map of blocks, in bytes */
    int bit_map_low;      /* byte offset into the bit map below which there are no
                             free blocks */
    byte * bit_map0;      /* the initial state of the bit map of blocks: 1 means in
                             use, 0 means free */
    byte * bit_map;       /* the current state of the bit map of blocks */
    byte * base;          /* for writing back as file baseA or baseB */
    int other_base_letter;/* - and the value 'B' or 'A' of the next base */
    char * name;          /* buffer that begins with the name of the directory holding the B tree */
    int name_len;         /* - and the length of the name within the buffer */
    int seq_count;        /* count of the number of successive instances of purely sequential
                             addition, starting at SEQ_START_POINT (neg) and going up to zero */
    int4 changed_n;       /* the last block to be changed by an addition */
    int changed_c;        /* - and the corresponding directory offset */
    int max_item_size;    /* maximum size of an item (key-tag pair) */
    int shared_level;     /* in B-tree read mode, cursors share blocks in
                             BC->C for levels at or above B->shared_level */
    char Btree_modified;  /* set to true the first time the B-tree is written to */
    char full_compaction; /* set to true when full compaction is to be achieved */

    int (* prev)(struct Btree *, struct Cursor *, int);
    int (* next)(struct Btree *, struct Cursor *, int);

                          /* B-tree navigation functions */

    struct Cursor C[BTREE_CURSOR_LEVELS];

};

struct Btree_item {

    int key_size;       /* capacity of item->key */
    int key_len;        /* length of retrieved key */
    byte * key;         /* pointer to the key */

    int tag_size;       /* capacity of item->tag */
    int tag_len;        /* length of retrieved tag */
    byte * tag;         /* pointer to the tag */

};

extern int Btree_find_key(struct Btree * B, byte * key, int key_len);
extern struct Btree_item * Btree_item_create();
extern int Btree_find_tag(struct Btree * B, byte * key, int key_len, struct Btree_item * t);
extern void Btree_item_lose(struct Btree_item * kt);
extern int Btree_add(struct Btree * B, byte * key, int key_len,
                                       byte * tag, int tag_len);
extern int Btree_delete(struct Btree * B, byte * key, int key_len);
extern struct Btree * Btree_open_to_write(char * name);
extern struct Btree * Btree_open_to_write_revision(char * name, unsigned long revision);
extern void Btree_quit(struct Btree * B);
extern void Btree_close(struct Btree * B, unsigned long revision);
extern void Btree_create(char * name, int block_size);
extern void Btree_check(char * name, char * opt_string);
extern struct Btree * Btree_open_to_read(char * name);
extern struct Btree * Btree_open_to_read_revision(char * name, unsigned long revision);
extern struct Bcursor * Bcursor_create(struct Btree * B);
extern int Bcursor_find_key(struct Bcursor * BC, byte * key, int key_len);
extern int Bcursor_next(struct Bcursor * BC);
extern int Bcursor_prev(struct Bcursor * BC);
extern int Bcursor_get_key(struct Bcursor * BC, struct Btree_item * kt);
extern int Bcursor_get_tag(struct Bcursor * BC, struct Btree_item * kt);
extern void Bcursor_lose(struct Bcursor * BC);
extern void Btree_full_compaction(struct Btree * B, int parity);

#ifdef __cplusplus
}
#endif

#endif /* OM_HGUARD_BTREE_H */
