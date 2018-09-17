#include <objectqueue.h>

bool ObjectQueue::defaultItemComparer(ObjectQueue::Item *a, ObjectQueue::Item *b)
{
    return a == b;
}

ObjectQueue::ObjectQueue() :
    first(nullptr)
{
}

void ObjectQueue::Add(ObjectQueue::Item *item, bool prepend)
{
    if(prepend)
    {
        item->Next = first;
        first = item;
        return;
    }

    item->Next = nullptr;
    if(!first)
    {
        first = item;
        return;
    }
    Item *it;
    for(it = first; it->Next; it = it->Next);
    it->Next = item;
}

ObjectQueue::Item *ObjectQueue::Get()
{
    if(!first)
        return nullptr;
    Item *it = first;
    first = it->Next;
    it->Next = nullptr;
    return it;
}

bool ObjectQueue::Remove(ObjectQueue::Item *item, ObjectQueue::ItemComparer comparer)
{
    if(!first) return false;
    if(!comparer) comparer = defaultItemComparer;

    if(comparer(first, item))
    {
        first = item->Next;
        item->Next = nullptr;
        return true;
    }

    Item *prev = nullptr;
    for(Item *it = first; it; it = it->Next)
    {
        if(comparer(it, item))
        {
            if(prev)
                prev->Next = it->Next;
            it->Next = nullptr;
            return true;
        }
        prev = it;
    }
    return false;
}

void ObjectQueue::ForEach(ObjectQueue::ForEachCallback action)
{
    if(!action) return;
    for(Item *it = first; it;)
        it = action(it) ? first : it->Next;
}

void ObjectQueue::Clear()
{
    while(Get());
}
