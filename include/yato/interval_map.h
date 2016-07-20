/**
* YATO library
*
* The MIT License (MIT)
* Copyright (c) 2016 Alexey Gruzdev
*/

#ifndef _YATO_INTERVAL_MAP_H_
#define _YATO_INTERVAL_MAP_H_

#include <map>
#include <limits>

namespace yato
{
    /**
     *  Map storing the whole range of keys in the form of intervals
     */
    template <typename _Key, typename _Value, class _Compare = std::less<_Key>, class _Allocator = std::allocator<std::pair<const _Key, _Value>>>
    class interval_map
    {
    public:
        using key_type = _Key;
        using mapped_type = _Value;
        using value_type = std::pair<const _Key, _Value>;
        using key_compare = _Compare;
        using allocator_type = _Allocator;

    private:
        std::map<_Key, _Value> m_map;
        //-------------------------------------------------------

        template <typename _ValRef>
        void _init_whole_range(_ValRef && val)
        {
            m_map.emplace_hint(m_map.begin(), std::numeric_limits<_Key>::min(), std::forward<_ValRef>(val));
        }
        //-------------------------------------------------------

    public:
        /**
         * Constructor associates whole range of _Key with val
         */
        explicit
        interval_map(const mapped_type & val)
        {
            _init_whole_range(val);
        };

        /**
         * Constructor associates whole range of _Key with val
         */
        explicit
        interval_map(mapped_type && val)
        {
            _init_whole_range(std::move(val));
        };

        /**
         * Constructor associates whole range of _Key with val
         */
        interval_map(const mapped_type & val, const key_compare & comparator, const allocator_type & allocator = allocator_type())
            : m_map(comparator, allocator)
        {
            _init_whole_range(val);
        };

        /**
         * Constructor associates whole range of _Key with val
         */
        interval_map(mapped_type && val, const key_compare & comparator, const allocator_type & allocator = allocator_type())
            : m_map(comparator, allocator)
        {
            _init_whole_range(std::move(val));
        };

        /**
         * Assign value val to interval [keyBegin, keyEnd). 
         * Overwrite previous values in this interval. 
         * Do not change values outside this interval.
         * If !( keyBegin < keyEnd ), this designates an empty interval, 
         * and assign must do nothing.
         */
        void assign(const key_type & keyBegin, const key_type & keyEnd, const mapped_type & val)
        {
            if (keyBegin < keyEnd) {
                //Find position for the end point
                auto endIt = m_map.lower_bound(keyEnd);
                bool endFound = false;
                if (endIt != m_map.end()) //check if the next interval has the same value
                {
                    //If the next point has the same key and same value 
                    //then we don't have to insert any new point
                    if (false == (endIt->first < keyEnd || keyEnd < endIt->first) && endIt->second == val) {
                        ++endIt;
                        endFound = true;
                    }
                }
                else //If the last interval has a value equal to 'val',
                {    //then we don't have to insert any new point
                    auto tempIt = endIt;
                    if ((--tempIt)->second == val) {
                        endFound = true;
                    }
                }
                //If an interval with the same value wasn't found, then check the current position
                if (false == endFound) {
                    auto tempIt = endIt;
                    --tempIt;
                    //Insert new point if 'keyEnd' doesn't exist or 
                    //the previous interval's value is not equal to 'val'.
                    //If the end key exists then the intervals [keyBegin, keyEnd) and [keyEnd, <some key>) 
                    //join tightly and no changes are needed
                    if (false == ((endIt != m_map.end() && false == (endIt->first < keyEnd || keyEnd < endIt->first)) || (tempIt->second == val))) {
                        endIt = m_map.emplace_hint(endIt, keyEnd, tempIt->second);
                    }
                }
                //Find position for the begin point
                auto beginIt = m_map.lower_bound(keyBegin);
                bool beginFound = false;
                //Check if the previous interval's value is equal to 'val'
                if (beginIt != m_map.begin()) {
                    auto beginPrev = beginIt;
                    --beginPrev;
                    if (beginPrev->second == val) {
                        //The begin point shifts to the previous interval's begin
                        //We don't have to insert any new point
                        beginIt = beginPrev;
                        beginFound = true;
                    }
                }
                //If an interval with the same value wasn't found, then check the current position
                if (false == beginFound) {
                    //If there is there is a point with the same key, then only change the value
                    if (beginIt != m_map.end() && false == (beginIt->first < keyBegin || keyBegin < beginIt->first)) {
                        beginIt->second = val;
                    }
                    else //otherwise insert new point
                    {
                        beginIt = m_map.emplace_hint(beginIt, keyBegin, val);
                    }
                }
                //Erase all points covered by the new interval
                m_map.erase(++beginIt, endIt);
            }
        }

        /**
         * Look-up of the value associated with key
         */
        const mapped_type & operator[](const key_type & key) const
        {
            return (--m_map.upper_bound(key))->second;
        }
    };
}

#endif
