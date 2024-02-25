#ifndef SORTEDMAP_H
#define SORTEDMAP_H


#include <QtCore/qmap.h>
#include <QtCore/qhash.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qglobal.h>

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------

//template<typename Key, typename T> class SortedMapIterator;

template<typename Key, typename T>
class SortedMap
{
    //friend class SortedMapIterator<Key, T>;
public:
    SortedMap();
    SortedMap(const SortedMap<Key, T> &other);
    SortedMap(SortedMap<Key, T> &&other);
    ~SortedMap();
    void clear();
    bool contains(const Key &key) const;
    T take(const Key &key);
    T &value(const Key &key);
    const T value(const Key &key,
                  const T &defaultValue = T()) const;
    T &valueByIndex(int index);
    const T valueByIndex(int index) const;
    void setValue(int index, const T &value);
    QList<T> values() const;
    QList<T> values(const Key &key) const;
    bool operator!=(const SortedMap<Key, T> &other) const;
    SortedMap<Key, T> & operator=(
            const SortedMap<Key, T> &other);
    SortedMap<Key, T> & operator=(
            SortedMap<Key, T> &&other);
    bool operator==(const SortedMap<Key, T> &other) const;
    T & operator[](const Key &key);
    const T operator[](const Key &key) const;
    bool empty() const;
    bool isEmpty() const;
    T & first();
    const T & first() const;
    void insert(const Key &key, const T &value);
    void replaceKey(const Key &oldKey, const Key &newKey);
    const Key key(
            const T &value,
            const Key &defaultKey = Key()) const;
    const Key keyByIndex(int index) const;
    const Key *keyIt(int index) const;
    QList<Key> keys() const;
    QList<Key> keys(const T &value) const;
    T & last();
    const T & last() const;
    const Key & firstKey() const;
    int remove(const Key &key);
    int remove(int index);
    int size() const;

protected:
    QList<Key> m_sortedKeys;
    QHash<Key, T> m_hash;


};
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
template<typename Key, typename T>
SortedMap<Key, T>::SortedMap()
{
}
//----------------------------------------------------------
template<typename Key, typename T>
SortedMap<Key, T>::SortedMap(const SortedMap<Key, T> &other)
{
    m_hash = other.m_hash;
    m_sortedKeys = other.m_sortedKeys;
}
//----------------------------------------------------------
template<typename Key, typename T>
SortedMap<Key, T>::SortedMap(SortedMap<Key, T> &&other)
{
    m_hash = qMove(other.m_hash);
    m_sortedKeys = qMove(other.m_sortedKeys);
}
//----------------------------------------------------------
template<typename Key, typename T>
SortedMap<Key, T> &SortedMap<Key, T>::operator=(
        const SortedMap<Key, T> &other)
{
    m_hash = other.m_hash;
    m_sortedKeys = other.m_sortedKeys;
    return *this;
}
//----------------------------------------------------------
template<typename Key, typename T>
SortedMap<Key, T> &SortedMap<Key, T>::operator=(
        SortedMap<Key, T> &&other)
{
    m_hash = qMove(other.m_hash);
    m_sortedKeys = qMove(other.m_sortedKeys);
    return *this;
}
//----------------------------------------------------------
template<typename Key, typename T>
SortedMap<Key, T>::~SortedMap()
{
}
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMap<Key, T>::operator!=(
        const SortedMap<Key, T> &other) const
{
    bool different
            = m_hash != other.m_hash
            || m_sortedKeys != other.m_sortedKeys;
    return different;
}
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMap<Key, T>::operator==(
        const SortedMap<Key, T> &other) const
{
    bool equals
            = m_hash == other.m_hash
            && m_sortedKeys == other.m_sortedKeys;
    return equals;
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMap<Key, T>::clear()
{
    m_sortedKeys.clear();
    m_hash.clear();
}
//----------------------------------------------------------
template<typename Key, typename T>
T &SortedMap<Key, T>::operator[](const Key &key)
{
    if(!m_sortedKeys.contains(key))
    {
        m_sortedKeys << key;
    }
    return m_hash[key];
}
//----------------------------------------------------------
template<typename Key, typename T>
const T SortedMap<Key, T>::operator[](const Key &key) const
{
    return m_hash[key];
}
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMap<Key, T>::contains(const Key &key) const
{
    bool contains = m_hash.contains(key);
    return contains;
}
//----------------------------------------------------------
template<typename Key, typename T>
T SortedMap<Key, T>::take(const Key &key)
{
    m_sortedKeys.removeOne(key);
    return m_hash.take(key);
}
//----------------------------------------------------------
template<typename Key, typename T>
T &SortedMap<Key, T>::value(const Key &key)
{
    return m_hash[key];
}
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMap<Key, T>::empty() const
{
    bool is = m_sortedKeys.isEmpty();
    return is;
}
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMap<Key, T>::isEmpty() const
{
    bool is = m_sortedKeys.isEmpty();
    return is;
}
//----------------------------------------------------------
template<typename Key, typename T>
T &SortedMap<Key, T>::first()
{
    return m_hash[m_sortedKeys[0]];
}
//----------------------------------------------------------
template<typename Key, typename T>
const T &SortedMap<Key, T>::first() const
{
    return m_hash[m_sortedKeys[0]];
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMap<Key, T>::insert(
        const Key &key, const T &value)
{
    m_sortedKeys << key;
    m_hash[key] = value;
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMap<Key, T>::replaceKey(
        const Key &oldKey, const Key &newKey)
{
    m_hash[newKey] = m_hash[oldKey];
    m_hash.remove(oldKey);
    int indexOld = m_sortedKeys.indexOf(oldKey);
    m_sortedKeys[indexOld] = newKey;
}
//----------------------------------------------------------
template<typename Key, typename T>
const Key SortedMap<Key, T>::key(
        const T &value,
        const Key &defaultKey) const
{
    return m_hash.key(value, defaultKey);
}
//----------------------------------------------------------
template<typename Key, typename T>
const Key SortedMap<Key, T>::keyByIndex(int index) const
{
    return m_sortedKeys[index];
}
//----------------------------------------------------------
template<typename Key, typename T>
const Key *SortedMap<Key, T>::keyIt(int index) const
{
    return &m_sortedKeys[index];
}
//----------------------------------------------------------
template<typename Key, typename T>
QList<Key> SortedMap<Key, T>::keys() const
{
    return m_sortedKeys;
}
//----------------------------------------------------------
template<typename Key, typename T>
QList<Key> SortedMap<Key, T>::keys(const T &value) const
{
    QList<Key> keys;
    keys << key(value);
    return keys;
}
//----------------------------------------------------------
template<typename Key, typename T>
T &SortedMap<Key, T>::last()
{
    return m_hash[m_sortedKeys.last()];
}
//----------------------------------------------------------
template<typename Key, typename T>
const T &SortedMap<Key, T>::last() const
{
    return m_hash[m_sortedKeys.last()];
}
//----------------------------------------------------------
template<typename Key, typename T>
const Key &SortedMap<Key, T>::firstKey() const
{
    return m_sortedKeys[0];
}
//----------------------------------------------------------
template<typename Key, typename T>
int SortedMap<Key, T>::remove(const Key &key)
{
    m_sortedKeys.removeAll(key);
    int ret = m_hash.remove(key);
    return ret;
}
//----------------------------------------------------------
template<typename Key, typename T>
int SortedMap<Key, T>::remove(int index) {
    int ret = remove(m_sortedKeys[index]);
    return ret;
}
//----------------------------------------------------------
template<typename Key, typename T>
int SortedMap<Key, T>::size() const
{
    int size = m_sortedKeys.size();
    return size;
}
//----------------------------------------------------------
template<typename Key, typename T>
const T SortedMap<Key, T>::valueByIndex(int index) const
{
    return m_hash[m_sortedKeys[index]];
}
//----------------------------------------------------------
template<typename Key, typename T>
T &SortedMap<Key, T>::valueByIndex(int index)
{
    return m_hash[m_sortedKeys[index]];
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMap<Key, T>::setValue(int index, const T &value)
{
    m_hash[m_sortedKeys[index]] = value;
}
//----------------------------------------------------------
template<typename Key, typename T>
const T SortedMap<Key, T>::value(
        const Key &key, const T &defaultValue) const
{
    T value = defaultValue;
    if(m_hash.contains(key))
    {
        value = m_hash[key];
    }
    return value;
}
//----------------------------------------------------------
template<typename Key, typename T>
QList<T> SortedMap<Key, T>::values() const
{
    QList<T> values;
    for(auto key : m_sortedKeys)
    {
        values << m_hash[key];
    }
    return values;
}
//----------------------------------------------------------
template<typename Key, typename T>
QList<T> SortedMap<Key, T>::values(const Key &key) const
{
    QList<T> values;
    values << value(key);
    return values;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
template<typename Key, typename T>
class SortedMapIterator
{
public:
    SortedMapIterator(const SortedMap<Key, T> &other);
    //SortedMapIterator &operator=(
            //const SortedMap<Key, T> &map);
    bool hasNext() const;
    bool hasPrevious() const;
    const Key key() const;
    void next();
    void previous();
    void toBack();
    void toFront();
    const T value() const;

private:
    const SortedMap<Key, T> *m_map;
    int m_position;
};
//----------------------------------------------------------
template<typename Key, typename T>
SortedMapIterator<Key, T>::SortedMapIterator(
        const SortedMap<Key, T> &other)
{
    m_map = &other;
    m_position = -1;
}

//----------------------------------------------------------
/*
template<typename Key, typename T>
SortedMapIterator &SortedMapIterator<Key, T>::operator=(
        const SortedMap<Key, T> &map)
{
    m_map = &map;
    m_position = -1;
    return *this;
}
//*/
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMapIterator<Key, T>::hasNext() const
{
    bool has = (m_position == -1 && m_map->size() > 0)
            || (m_position < m_map->size() - 1);
    return has;
}
//----------------------------------------------------------
template<typename Key, typename T>
bool SortedMapIterator<Key, T>::hasPrevious() const
{
    bool has = m_map->size() > 1 && m_position > 0;
    return has;
}
//----------------------------------------------------------
template<typename Key, typename T>
const Key SortedMapIterator<Key, T>::key() const
{
    return m_map->keyByIndex(m_position);
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMapIterator<Key, T>::next()
{
    if(m_position < m_map->size() - 1)
    {
        m_position++;
    }
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMapIterator<Key, T>::previous()
{
    if(m_position > 0)
    {
        m_position--;
    }
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMapIterator<Key, T>::toBack()
{
    m_position = m_map->size() - 1;
}
//----------------------------------------------------------
template<typename Key, typename T>
void SortedMapIterator<Key, T>::toFront()
{
    if(m_map->size() > 0)
    {
        m_position = 0;
    }
    else
    {
        m_position = -1;
    }
}
//----------------------------------------------------------
template<typename Key, typename T>
const T SortedMapIterator<Key, T>::value() const
{
    return m_map->value(m_position);
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------

#endif // SORTEDMAP_H
