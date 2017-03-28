
#ifndef __MEMORY_TRACKER__H__
#define __MEMORY_TRACKER__H__

template <typename _TrackedType>
class MemoryTracker
{
public:
    static size_t BytesUsed(void);

protected:
    MemoryTracker(void);
    MemoryTracker(const MemoryTracker<_TrackedType>& rhs);
    virtual ~MemoryTracker(void);

private:
    size_t ObjectSize(void);
    static size_t mByte;
};

template <typename _TackedType>
size_t MemoryTracker<_TackedType>::BytesUsed(void)
{
    return MemoryTracker<_TackedType>::mByte;
}

template <typename _TackedType>
MemoryTracker<_TackedType>::MemoryTracker(void)
{
    MemoryTracker<_TackedType>::mByte += ObjectSize();
}

template <typename _TackedType>
MemoryTracker<_TackedType>::MemoryTracker(const MemoryTracker<_TackedType>& rhs)
{
    MemoryTracker<_TackedType>::mByte += ObjectSize();
}

template <typename _TrackedType>
MemoryTracker<_TrackedType>::~MemoryTracker(void)
{
    MemoryTracker<_TrackedType>::mByte -= ObjectSize();
}

template <typename _TrackedType>
size_t MemoryTracker<_TrackedType>::ObjectSize(void)
{
    return sizeof(*static_cast<_TrackedType*> (this));
}

template <typename _TrackedType>
size_t MemoryTracker<_TrackedType>::mByte = 0;

#endif /*__MEMORY_TRACKER__H__*/