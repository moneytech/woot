#ifndef OBJECTQUEUE_H
#define OBJECTQUEUE_H

#include <types.h>

class ObjectQueue
{
public:
    class Item
    {
        friend class ObjectQueue;
        Item *Next;
    };
    typedef bool (*ItemComparer)(Item *a, Item *b);
    typedef bool (*ForEachCallback)(Item *a); // return true to restart the loop
private:
    Item *first;
    static bool defaultItemComparer(Item *a, Item *b);
public:
    ObjectQueue();
    void Add(Item *item, bool prepend);
    Item *Get();
    bool Remove(Item *item, ItemComparer comparer);
    void ForEach(ForEachCallback action);
    void Clear();
};

#endif // OBJECTQUEUE_H
