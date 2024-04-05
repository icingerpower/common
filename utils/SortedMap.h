#ifndef SORTEDMAP_H
#define SORTEDMAP_H

#include <QMap>
#include <QHash>
#include <QStringList>
#include <QtGlobal>

/*
 * Classe qui fonctionne comme un QHash, mais où les éléments sont itérés par ordre d'insertion
 */

template<typename Key, typename T>
class SortedMap
{
public:
    SortedMap();
    SortedMap(const SortedMap<Key, T> &other);
    SortedMap(SortedMap<Key, T> &&other);
    SortedMap(std::initializer_list<std::pair<Key, T>> values);
    ~SortedMap();
    void clear();
    bool contains(const Key &key) const;
    T take(const Key &key);
    const T &valueByRef(const Key &key) const;
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
    
public:
    class const_iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const std::pair<Key, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = const std::pair<Key, T>*;
        using reference = const std::pair<Key, T>&;
 
        const_iterator(typename QList<Key>::const_iterator keyIt, const QHash<Key, T>& hash)
            : keyIt(keyIt), hash(hash) {}
 
        const_iterator& operator++() { ++keyIt; return *this; }
        const_iterator operator++(int) { const_iterator temp(*this); ++keyIt; return temp; }
        const_iterator& operator--() { --keyIt; return *this; }
        const_iterator operator--(int) { const_iterator temp(*this); --keyIt; return temp; }
        bool operator==(const const_iterator& other) const { return keyIt == other.keyIt; }
        bool operator!=(const const_iterator& other) const { return keyIt != other.keyIt; }
        const std::pair<Key, T>& operator*() const { return std::make_pair(*keyIt, hash.value(*keyIt)); }
 
        const Key& key() const { return *keyIt; }
        const T &value() const { return hash.constFind(*keyIt).value(); }
 
    private:
        typename QList<Key>::const_iterator keyIt;
        const QHash<Key, T>& hash;
    };
    
    const_iterator constBegin() const { return const_iterator(m_sortedKeys.constBegin(), m_hash); }
    const_iterator constEnd() const { return const_iterator(m_sortedKeys.constEnd(), m_hash); }
    
    class iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::pair<Key, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        
        iterator(typename QList<Key>::iterator keyIt, QHash<Key, T>& hash)
            : keyIt(keyIt), hash(hash) {}
        
        iterator& operator++() { ++keyIt; return *this; }
        iterator operator++(int) { iterator temp(*this); ++keyIt; return temp; }
        iterator& operator--() { --keyIt; return *this; }
        iterator operator--(int) { iterator temp(*this); --keyIt; return temp; }
        bool operator==(const iterator& other) const { return keyIt == other.keyIt; }
        bool operator!=(const iterator& other) const { return keyIt != other.keyIt; }
        std::pair<Key, T>& operator*() { return std::make_pair(*keyIt, hash[*keyIt]); }
        
        Key& key() { return *keyIt; }
        T& value() { return hash[*keyIt]; }
        
    private:
        typename QList<Key>::iterator keyIt;
        QHash<Key, T>& hash;
    };
    
    iterator begin() { return iterator(m_sortedKeys.begin(), m_hash); }
    iterator end() { return iterator(m_sortedKeys.end(), m_hash); }
};

template<typename Key, typename T>
SortedMap<Key, T>::SortedMap()
{
}

template<typename Key, typename T>
SortedMap<Key, T>::SortedMap(const SortedMap<Key, T> &other)
{
    m_hash = other.m_hash;
    m_sortedKeys = other.m_sortedKeys;
}

template<typename Key, typename T>
SortedMap<Key, T>::SortedMap(SortedMap<Key, T> &&other)
{
    m_hash = qMove(other.m_hash);
    m_sortedKeys = qMove(other.m_sortedKeys);
}

template<typename Key, typename T>
SortedMap<Key, T>::SortedMap(
    std::initializer_list<std::pair<Key, T> > values)
{
    for (const auto &value : values)
    {
        m_sortedKeys << value.first;
        m_hash[value.first] = value.second;
    }
}

template<typename Key, typename T>
SortedMap<Key, T> &SortedMap<Key, T>::operator=(
        const SortedMap<Key, T> &other)
{
    m_hash = other.m_hash;
    m_sortedKeys = other.m_sortedKeys;
    return *this;
}

template<typename Key, typename T>
SortedMap<Key, T> &SortedMap<Key, T>::operator=(
        SortedMap<Key, T> &&other)
{
    m_hash = qMove(other.m_hash);
    m_sortedKeys = qMove(other.m_sortedKeys);
    return *this;
}

template<typename Key, typename T>
SortedMap<Key, T>::~SortedMap()
{
}

template<typename Key, typename T>
bool SortedMap<Key, T>::operator!=(
        const SortedMap<Key, T> &other) const
{
    bool different
            = m_hash != other.m_hash
            || m_sortedKeys != other.m_sortedKeys;
    return different;
}

template<typename Key, typename T>
bool SortedMap<Key, T>::operator==(
        const SortedMap<Key, T> &other) const
{
    bool equals
            = m_hash == other.m_hash
            && m_sortedKeys == other.m_sortedKeys;
    return equals;
}

template<typename Key, typename T>
void SortedMap<Key, T>::clear()
{
    m_sortedKeys.clear();
    m_hash.clear();
}

template<typename Key, typename T>
T &SortedMap<Key, T>::operator[](const Key &key)
{
    if(!m_sortedKeys.contains(key))
    {
        m_sortedKeys << key;
    }
    return m_hash[key];
}

template<typename Key, typename T>
const T SortedMap<Key, T>::operator[](const Key &key) const
{
    return m_hash[key];
}

template<typename Key, typename T>
bool SortedMap<Key, T>::contains(const Key &key) const
{
    bool contains = m_hash.contains(key);
    return contains;
}

template<typename Key, typename T>
T SortedMap<Key, T>::take(const Key &key)
{
    m_sortedKeys.removeOne(key);
    return m_hash.take(key);
}

template<typename Key, typename T>
T &SortedMap<Key, T>::value(const Key &key)
{
    return m_hash[key];
}

template<typename Key, typename T>
bool SortedMap<Key, T>::empty() const
{
    bool is = m_sortedKeys.isEmpty();
    return is;
}

template<typename Key, typename T>
bool SortedMap<Key, T>::isEmpty() const
{
    bool is = m_sortedKeys.isEmpty();
    return is;
}

template<typename Key, typename T>
T &SortedMap<Key, T>::first()
{
    return m_hash[m_sortedKeys[0]];
}

template<typename Key, typename T>
const T &SortedMap<Key, T>::first() const
{
    return m_hash[m_sortedKeys[0]];
}

template<typename Key, typename T>
void SortedMap<Key, T>::insert(
        const Key &key, const T &value)
{
    m_sortedKeys << key;
    m_hash[key] = value;
}

template<typename Key, typename T>
void SortedMap<Key, T>::replaceKey(
        const Key &oldKey, const Key &newKey)
{
    m_hash[newKey] = m_hash[oldKey];
    m_hash.remove(oldKey);
    int indexOld = m_sortedKeys.indexOf(oldKey);
    m_sortedKeys[indexOld] = newKey;
}

template<typename Key, typename T>
const Key SortedMap<Key, T>::key(
        const T &value,
        const Key &defaultKey) const
{
    return m_hash.key(value, defaultKey);
}

template<typename Key, typename T>
const Key SortedMap<Key, T>::keyByIndex(int index) const
{
    return m_sortedKeys[index];
}

template<typename Key, typename T>
const Key *SortedMap<Key, T>::keyIt(int index) const
{
    return &m_sortedKeys[index];
}

template<typename Key, typename T>
QList<Key> SortedMap<Key, T>::keys() const
{
    return m_sortedKeys;
}

template<typename Key, typename T>
QList<Key> SortedMap<Key, T>::keys(const T &value) const
{
    QList<Key> keys;
    keys << key(value);
    return keys;
}

template<typename Key, typename T>
T &SortedMap<Key, T>::last()
{
    return m_hash[m_sortedKeys.last()];
}

template<typename Key, typename T>
const T &SortedMap<Key, T>::last() const
{
    return m_hash[m_sortedKeys.last()];
}

template<typename Key, typename T>
const Key &SortedMap<Key, T>::firstKey() const
{
    return m_sortedKeys[0];
}

template<typename Key, typename T>
int SortedMap<Key, T>::remove(const Key &key)
{
    m_sortedKeys.removeAll(key);
    int ret = m_hash.remove(key);
    return ret;
}

template<typename Key, typename T>
int SortedMap<Key, T>::remove(int index) {
    int ret = remove(m_sortedKeys[index]);
    return ret;
}

template<typename Key, typename T>
int SortedMap<Key, T>::size() const
{
    int size = m_sortedKeys.size();
    return size;
}

template<typename Key, typename T>
const T SortedMap<Key, T>::valueByIndex(int index) const
{
    return m_hash[m_sortedKeys[index]];
}

template<typename Key, typename T>
T &SortedMap<Key, T>::valueByIndex(int index)
{
    return m_hash[m_sortedKeys[index]];
}

template<typename Key, typename T>
void SortedMap<Key, T>::setValue(int index, const T &value)
{
    m_hash[m_sortedKeys[index]] = value;
}

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

template<typename Key, typename T>
const T &SortedMap<Key, T>::valueByRef(
        const Key &key) const
{
    return m_hash.constFind(key).value();
}

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

template<typename Key, typename T>
QList<T> SortedMap<Key, T>::values(const Key &key) const
{
    QList<T> values;
    values << value(key);
    return values;
}

#endif // SORTEDMAP_H
