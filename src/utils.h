#ifndef _UTILS_H
#define _UTILS_H

#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <atomic>
#include <cassert>
#include <cstring>
#include "atomic.h"

#include "rwlock.h"


namespace std {
	template<typename T, size_t N>
	class circular_queue {
	public:
		circular_queue(int size = N, T value = T()) :
			_data(size + 1, value), _tail(0), _front(0),
			_tail_busy{ ATOMIC_FLAG_INIT }, _front_busy{ ATOMIC_FLAG_INIT } {

		}
		bool full() {
			return (_tail + 1) % _data.size() == _front;
		}
		bool empty() {
			return (_tail == _front);
		}
		bool push(T &value) {
			while (_tail_busy.test_and_set())
				;
			if (full()) {
				_tail_busy.clear();
				return false;
			}
			_data[_tail] = value;
			_tail = (_tail + 1) % _data.size();
			_tail_busy.clear();
			return true;
		}
		bool pop(T &value) {
			while (_front_busy.test_and_set())
				;

			if (empty()) {
				_front_busy.clear();
				return false;
			}

			value = _data[_front];
			_front = (_front + 1) % _data.size();
			_front_busy.clear();
			return true;
		}
		int size() {
			return _data.size() - 1;
		}
	private:
		std::vector<T> _data;
		size_t _tail;
		size_t _front;
		atomic_flag _tail_busy;
		atomic_flag _front_busy;

	};

	template<typename T>
	class buffer {
	public:
		using size_t = std::size_t;

		buffer(size_t bsize = 1000) :
			_buffer(bsize), _buffer_list(0), _buffer_busy{ ATOMIC_FLAG_INIT } {
			_init();
		}

		T* require() {
			while (_buffer_busy.test_and_set())
				;
			if (full()) {
				_buffer_busy.clear();
				return nullptr;
			}
			_node *nd = &_buffer[_buffer_list];
			_buffer_list = nd->next;
			_buffer_busy.clear();
            acess_ok(&(nd->data));
			return &(nd->data);
		}
		void release(T *data) {
			_node *nd = reinterpret_cast<_node *> (reinterpret_cast<size_t>(data) - offset_of_data());
			if (nd < &_buffer[0] || nd >= &_buffer[0] + _buffer.size()) {
                assert(false);
				return;
            }

			while (_buffer_busy.test_and_set())
				;
			nd->next = _buffer_list;
			_buffer_list = nd - &_buffer[0];

			_buffer_busy.clear();
		}

        bool acess_ok(void *p) {
            assert(_access_ok(p));
            return true;
        }

        bool _access_ok(void *p) {
			return (!(p < &_buffer[0] || p >= &_buffer[0] + _buffer.size()));
        }

	private:
		size_t offset_of_data() {
			return (size_t) &( ((_node *)0) -> data);
		}
		void _init() {
            memset(&_buffer[0], 0, sizeof(_node) * _buffer.size());
			for (size_t i = 0; i < _buffer.size(); ++i)
				_buffer[i].next = i + 1;
		}
		bool full() {
			return _buffer_list >= _buffer.size();
		}

		struct _node {
			uint64_t next;
			T data;
		};
				
		std::vector<_node> _buffer;
		size_t _buffer_list;
		atomic_flag _buffer_busy;

	};

    // multiple readers and single writer
    template<typename T>
    class buffered_list {
            struct _node {
                struct _node *next;
                struct _node *prev;
                T data;
            } __attribute__((aligned(8)));
        public:
            using node_type = _node;
            using compare_type = bool(*)(const T& d1, const T& d2);

            buffered_list(buffer<node_type> *bf = nullptr): 
                _writer_busy{ATOMIC_FLAG_INIT},
                _rwlock(PTHREAD_RWLOCK_INITIALIZER),
                _mem(bf) {

                _list.next = &_list;
                _list.prev = &_list;
            }


            void rlock() {
                //pthread_rwlock_rdlock(&_rwlock);
                _rwlock2.RDLock();
            }
            void wlock() {
                //pthread_rwlock_wrlock(&_rwlock);
                _rwlock2.WRLock();
            }
            void wunlock() {
                //pthread_rwlock_unlock(&_rwlock);
                _rwlock2.WRUnLock();
            }

            void runlock() {
                _rwlock2.RDUnLock();
            }

            void lock() {
                while (_writer_busy.test_and_set())
                    ;
            }
            void unlock() {
                _writer_busy.clear();
            }

            T* begin() {
                return &(_list.next->data);
            }
            T* end() {
                return &(_list.data);
            }
            T* next(T *data) {
                access_ok(data);
                auto offset = (char *)&(((node_type *)0)->data);
                auto *p = (node_type *)((char *)data - offset);
                T *pnext = nullptr;

                //lock();
                assert(_mem->_access_ok(p->next) || p->next == &_list);
                pnext = &(p->next->data);
                //unlock();
                return pnext;
            }

            bool access_ok(void* data) {
                return _mem->acess_ok(data);
            }


            bool set_buffer(buffer<node_type> *bf) {
                if (!_mem) {
                    _mem = bf;
                    return true;
                }
                return false;
            }


            T* push_front(const T& data) {

                node_type *nd = _mem->require();
                if (!nd) return nullptr;

                access_ok(nd);
                nd->data = data;

                wlock();

                //auto *p = _list.next;

                nd->next = _list.next;
                nd->prev = &_list;
                _list.next->prev = nd;
                _list.next = nd;
                /*
                atomic_set(&(nd->next), _list.next);
                atomic_set(&(nd->prev), &_list);
                __sync_synchronize();
                atomic_set(&(_list.next->prev), nd);
                atomic_set(&(_list.next), nd);
                */

                access_ok(nd);

                //assert(_mem->_access_ok(_list.next) || _list.next == &_list);
                //assert(_mem->_access_ok(nd->next) || nd->next == &_list);
                //assert(_mem->_access_ok(p->next) || p->next == &_list);

                wunlock();

                return &(nd->data);
            }

            void remove(const T* pdata) {
                auto offset = (char *)&(((node_type *)0)->data);
                auto *p = (node_type *)((char *)pdata - offset);
                //auto *pprev = p->prev;
                //auto *pnext = p->next;

                wlock();

                p->prev->next = p->next;
                p->next->prev = p->prev;
                /*
                atomic_set(&(p->prev->next), p->next);
                atomic_set(&(p->next->prev), p->prev);
                */

                wunlock();

                _mem->release(p);
            }

            void remove(const T& data) {
                node_type *nd = _find(data);
                if (nd) {

                    wlock();

                    nd->prev->next = nd->next;
                    nd->next->prev = nd->prev;

                    wunlock();

                    access_ok(nd);
                    assert(_mem->_access_ok(nd->next) || nd->next == &_list);
                    _mem->release(nd);
                }
            }

            T *find(const T& data, compare_type equal) {

                lock();
                node_type *nd = _find(data, equal);
                access_ok(nd);
                unlock();

                if (nd)
                    return &(nd->data);
                return nullptr;
            }

        private:
            static bool def_equal(const T& d1, const T& d2) {
                return memcmp(&d1, &d2, sizeof(T)) == 0;
            }

            node_type* _find(const T& data, compare_type equal = &(buffered_list::def_equal)) {
                node_type *nd =  _list.next;
                for(; nd != &_list ; nd = nd->next) {
                    if (equal(nd->data, data))
                        return nd;
                }
                return nullptr;
            }

		    atomic_flag _writer_busy;
            pthread_rwlock_t _rwlock;
            RWLock _rwlock2;
            buffer<node_type> *_mem;
            _node _list;

    };
};


#endif
