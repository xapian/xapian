
struct pool {

    int size;
    struct pool_entry * entries;

};

extern struct pool * create_pool(char * s[]);
extern char * search_pool(struct pool * p, int length, char * s);
extern void free_pool(struct pool * p);


