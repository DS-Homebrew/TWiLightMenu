/*
    sigslot.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// __________________________________________________________________________________________________
//  The original version of this code was written and placed in the public domain by Sarah Thompson
// See http://sigslot.sourceforge.net/
// __________________________________________________________________________________________________
#ifndef SIGSLOT__H
#define SIGSLOT__H

#include <list>
#include <set>

namespace akui
{

class SlotHolder;

class BasicConnection0
{
  public:
    virtual ~BasicConnection0() {}
    virtual SlotHolder *targetSlotHolder() const = 0;
    virtual void emit() = 0;
    virtual BasicConnection0 *clone() = 0;
    virtual BasicConnection0 *duplicate(SlotHolder *newTarget) = 0;
};

template <class Type1>
class BasicConnection1
{
  public:
    virtual ~BasicConnection1() {}
    virtual SlotHolder *targetSlotHolder() const = 0;
    virtual void emit(Type1) = 0;
    virtual BasicConnection1<Type1> *clone() = 0;
    virtual BasicConnection1<Type1> *duplicate(SlotHolder *newTarget) = 0;
};

class BasicSignal
{
  public:
    virtual ~BasicSignal() {}
    virtual void disconnectSlot(SlotHolder *slot) = 0;
    virtual void duplicateSlot(const SlotHolder *oldSlot, SlotHolder *newSlot) = 0;
};

class SlotHolder
{
  private:
    typedef std::set<BasicSignal *> SenderSet;
    typedef SenderSet::const_iterator const_iterator;

  public:
    SlotHolder() {}

    SlotHolder(const SlotHolder &holder)
    {
        const_iterator it = holder.senders_.begin();
        const_iterator itEnd = holder.senders_.end();

        while (it != itEnd)
        {
            (*it)->duplicateSlot(&holder, this);
            senders_.insert(*it);
            ++it;
        }
    }

    void connectTo(BasicSignal *sender)
    {
        senders_.insert(sender);
    }

    void disconnectFrom(BasicSignal *sender)
    {
        senders_.erase(sender);
    }

    virtual ~SlotHolder()
    {
        disconnectAll();
    }

    void disconnectAll()
    {
        const_iterator it = senders_.begin();
        const_iterator itEnd = senders_.end();

        while (it != itEnd)
        {
            (*it)->disconnectSlot(this);
            ++it;
        }

        senders_.erase(senders_.begin(), senders_.end());
    }

  protected:
    SenderSet senders_;
};

class BasicSignal0 : public BasicSignal
{
  public:
    typedef std::list<BasicConnection0 *> ConnectionList;

    BasicSignal0() {}

    BasicSignal0(const BasicSignal0 &s) : BasicSignal(s)
    {

        ConnectionList::const_iterator it = s.connectedSlots_.begin();
        ConnectionList::const_iterator itEnd = s.connectedSlots_.end();

        while (it != itEnd)
        {
            (*it)->targetSlotHolder()->connectTo(this);
            connectedSlots_.push_back((*it)->clone());

            ++it;
        }
    }

    ~BasicSignal0()
    {
        disconnectAll();
    }

    void disconnectAll()
    {

        ConnectionList::const_iterator it = connectedSlots_.begin();
        ConnectionList::const_iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            (*it)->targetSlotHolder()->disconnectFrom(this);
            delete *it;

            ++it;
        }

        connectedSlots_.erase(connectedSlots_.begin(), connectedSlots_.end());
    }

    void disconnect(SlotHolder *slotHolder)
    {
        ConnectionList::iterator it = connectedSlots_.begin();
        ConnectionList::iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            if ((*it)->targetSlotHolder() == slotHolder)
            {
                delete *it;
                connectedSlots_.erase(it);
                slotHolder->disconnectFrom(this);
                return;
            }

            ++it;
        }
    }

    void disconnectSlot(SlotHolder *slot)
    {
        ConnectionList::iterator it = connectedSlots_.begin();
        ConnectionList::iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            ConnectionList::iterator itNext = it;
            ++itNext;

            if ((*it)->targetSlotHolder() == slot)
            {
                delete *it;
                connectedSlots_.erase(it);
            }

            it = itNext;
        }
    }

    void duplicateSlot(const SlotHolder *oldTarget, SlotHolder *newTarget)
    {
        ConnectionList::iterator it = connectedSlots_.begin();
        ConnectionList::iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            if ((*it)->targetSlotHolder() == oldTarget)
            {
                connectedSlots_.push_back((*it)->duplicate(newTarget));
            }

            ++it;
        }
    }

  protected:
    ConnectionList connectedSlots_;
};

template <class Type1>
class BasicSignal1 : public BasicSignal
{
  public:
    typedef typename std::list<BasicConnection1<Type1> *> ConnectionList;

    BasicSignal1() {}

    BasicSignal1(const BasicSignal1<Type1> &s) : BasicSignal(s)
    {
        typename ConnectionList::const_iterator it = s.connectedSlots_.begin();
        typename ConnectionList::const_iterator itEnd = s.connectedSlots_.end();

        while (it != itEnd)
        {
            (*it)->targetSlotHolder()->connectTo(this);
            connectedSlots_.push_back((*it)->clone());

            ++it;
        }
    }

    void duplicateSlot(const SlotHolder *oldTarget, SlotHolder *newTarget)
    {

        typename ConnectionList::iterator it = connectedSlots_.begin();
        typename ConnectionList::iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {

            if ((*it)->targetSlotHolder() == oldTarget)
            {
                connectedSlots_.push_back((*it)->duplicate(newTarget));
            }

            ++it;
        }
    }

    ~BasicSignal1()
    {
        disconnectAll();
    }

    void disconnectAll()
    {

        typename ConnectionList::const_iterator it = connectedSlots_.begin();
        typename ConnectionList::const_iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            (*it)->targetSlotHolder()->disconnectFrom(this);
            delete *it;

            ++it;
        }

        connectedSlots_.erase(connectedSlots_.begin(), connectedSlots_.end());
    }

    void disconnect(SlotHolder *slotHolder)
    {

        typename ConnectionList::iterator it = connectedSlots_.begin();
        typename ConnectionList::iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            if ((*it)->targetSlotHolder() == slotHolder)
            {
                delete *it;
                connectedSlots_.erase(it);
                slotHolder->disconnectFrom(this);
                return;
            }

            ++it;
        }
    }

    void disconnectSlot(SlotHolder *slot)
    {

        typename ConnectionList::iterator it = connectedSlots_.begin();
        typename ConnectionList::iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            typename ConnectionList::iterator itNext = it;
            ++itNext;

            if ((*it)->targetSlotHolder() == slot)
            {
                delete *it;
                connectedSlots_.erase(it);
            }

            it = itNext;
        }
    }

  protected:
    ConnectionList connectedSlots_;
};

template <class TargetType>
class Connection0 : public BasicConnection0
{
  public:
    Connection0()
    {
        object_ = 0;
        memberFunction_ = 0;
        voidMemberFunction_ = 0;
    }

    Connection0(TargetType *anObject, void (TargetType::*aMemberFunction)())
    {
        object_ = anObject;
        memberFunction_ = 0;
        voidMemberFunction_ = aMemberFunction;
    }

    Connection0(TargetType *anObject, TargetType &(TargetType::*aMemberFunction)())
    {
        object_ = anObject;
        memberFunction_ = aMemberFunction;
    }

    virtual BasicConnection0 *clone()
    {
        return new Connection0<TargetType>(*this);
    }

    virtual BasicConnection0 *duplicate(SlotHolder *newTarget)
    {
        return new Connection0<TargetType>((TargetType *)newTarget, memberFunction_);
    }

    virtual void emit()
    {
        if (memberFunction_ != 0)
            (object_->*memberFunction_)();
        else
            (object_->*voidMemberFunction_)();
    }

    virtual SlotHolder *targetSlotHolder() const
    {
        return object_;
    }

  private:
    TargetType *object_;
    TargetType &(TargetType::*memberFunction_)();
    void (TargetType::*voidMemberFunction_)();
};

template <class TargetType, class Type1>
class Connection1 : public BasicConnection1<Type1>
{
  public:
    Connection1()
    {
        object_ = 0;
        memberFunction_ = 0;
        voidMemberFunction_ = 0;
    }

    Connection1(TargetType *anObject, void (TargetType::*aMemberFunction)(Type1))
    {
        object_ = anObject;
        memberFunction_ = 0;
        voidMemberFunction_ = aMemberFunction;
    }

    Connection1(TargetType *anObject, TargetType &(TargetType::*aMemberFunction)(Type1))
    {
        object_ = anObject;
        voidMemberFunction_ = 0;
        memberFunction_ = aMemberFunction;
    }

    virtual BasicConnection1<Type1> *clone()
    {
        return new Connection1<TargetType, Type1>(*this);
    }

    virtual BasicConnection1<Type1> *duplicate(SlotHolder *newTarget)
    {
        return new Connection1<TargetType, Type1>((TargetType *)newTarget, memberFunction_);
    }

    virtual void emit(Type1 a1)
    {
        if (memberFunction_ != 0)
            (object_->*memberFunction_)(a1);
        else
            (object_->*voidMemberFunction_)(a1);
    }

    virtual SlotHolder *targetSlotHolder() const
    {
        return object_;
    }

  private:
    TargetType *object_;
    void (TargetType::*voidMemberFunction_)(Type1);
    TargetType &(TargetType::*memberFunction_)(Type1);
};

class Signal0 : public BasicSignal0
{
  public:
    typedef BasicSignal0::ConnectionList ConnectionList;

  public:
    Signal0() {}

    Signal0(const Signal0 &s) : BasicSignal0(s) {}

    template <class TargetType>
    void connect(TargetType *slotHolder, void (TargetType::*aMemberFunction)())
    {
        Connection0<TargetType> *conn =
            new Connection0<TargetType>(slotHolder, aMemberFunction);
        connectedSlots_.push_back(conn);
        slotHolder->connectTo(this);
    }

    template <class TargetType>
    void connect(TargetType *slotHolder, TargetType &(TargetType::*aMemberFunction)())
    {
        Connection0<TargetType> *conn =
            new Connection0<TargetType>(slotHolder, aMemberFunction);
        connectedSlots_.push_back(conn);
        slotHolder->connectTo(this);
    }

    void emit()
    {
        ConnectionList::const_iterator itNext, it = connectedSlots_.begin();
        ConnectionList::const_iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            itNext = it;
            ++itNext;

            (*it)->emit();

            it = itNext;
        }
    }

    void operator()()
    {
        ConnectionList::const_iterator itNext, it = connectedSlots_.begin();
        ConnectionList::const_iterator itEnd = connectedSlots_.end();

        while (it != itEnd)
        {
            itNext = it;
            ++itNext;

            (*it)->emit();

            it = itNext;
        }
    }
    size_t size(void)
    {
        return connectedSlots_.size();
    }
};

template <class Type1>
class Signal1 : public BasicSignal1<Type1>
{
  public:
    typedef typename BasicSignal1<Type1>::ConnectionList ConnectionList;

  public:
    Signal1() {}

    Signal1(const Signal1<Type1> &s) : BasicSignal1<Type1>(s) {}

    template <class TargetType>
    void connect(TargetType *slotHolder, void (TargetType::*aMemberFunction)(Type1))
    {

        Connection1<TargetType, Type1> *conn =
            new Connection1<TargetType, Type1>(slotHolder, aMemberFunction);
        BasicSignal1<Type1>::connectedSlots_.push_back(conn);
        slotHolder->connectTo(this);
    }

    template <class TargetType>
    void connect(TargetType *slotHolder, TargetType &(TargetType::*aMemberFunction)(Type1))
    {

        Connection1<TargetType, Type1> *conn =
            new Connection1<TargetType, Type1>(slotHolder, aMemberFunction);
        BasicSignal1<Type1>::connectedSlots_.push_back(conn);
        slotHolder->connectTo(this);
    }

    void emit(Type1 a1)
    {

        typename ConnectionList::const_iterator itNext, it = BasicSignal1<Type1>::connectedSlots_.begin();
        typename ConnectionList::const_iterator itEnd = BasicSignal1<Type1>::connectedSlots_.end();

        while (it != itEnd)
        {
            itNext = it;
            ++itNext;

            (*it)->emit(a1);

            it = itNext;
        }
    }

    void operator()(Type1 a1)
    {

        typename ConnectionList::const_iterator itNext, it = BasicSignal1<Type1>::connectedSlots_.begin();
        typename ConnectionList::const_iterator itEnd = BasicSignal1<Type1>::connectedSlots_.end();

        while (it != itEnd)

        {
            itNext = it;
            ++itNext;

            (*it)->emit(a1);

            it = itNext;
        }
    }
    size_t size(void)
    {
        return BasicSignal1<Type1>::connectedSlots_.size();
    }
};

} // namespace akui

#endif // SIGSLOT__H
