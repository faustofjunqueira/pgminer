#ifndef TOPK_H
#define TOPK_H

#include "pallocable.h"
#include <exception>

template <class T>
class TopK : public PAllocable {
    
    public:
        
        class TopKException : public std::exception {
            public:
            TopKException(const char * const msg) : _msg(msg) { }
            virtual const char * what() const throw() { return _msg; }
            const char * _msg;
        };
        
    private:
    
        class Node : public PAllocable {
            public:
                Node() { }
                Node(const double w, const T d) : weight(w), data(d) { }
                double weight;
                T data;
        };
        
    public:
    
        TopK(const int k) : _k(k), _eltos(0), _mem( new Node[k] ) { }
        
        void push(const double weight, T data)
        {
            if (_eltos == _k) {
                if (_mem[0].weight < weight) {
                    _mem[0] = Node(weight, data);
                    goDown(0);
                }
            } else {
                _mem[_eltos] = Node(weight, data);
                goUp(_eltos++);
            }
        }
        
        T pop()
        {
            T t;
            double w;
            pop(t, w);
            return w;
        }
        
        void pop(double & w, T & t)
        {
            if (_eltos == 0)
                throw TopKException("TopK is empty.");
            
            t = _mem[0].data;
            w = _mem[0].weight;
            _mem[0] = _mem[_eltos-1];
            _eltos--;
            goDown(0);
        }
        
        int size() const
        {
            return _eltos;
        }
        
        int k() const
        {
            return _k;
        }
        
        void clear()
        {
            _eltos = 0;
        }
        
    private:
        
        void goUp(int i) 
        {
            Node temp;
            int parentId = parent(i);
            
            while(i != 0 && _mem[parentId].weight > _mem[i].weight) {
                temp = _mem[parentId];
                _mem[parentId] = _mem[i];
                _mem[i] = temp;
                i = parentId;
            }
        }
        
        void goDown(int i)
        {
            Node temp;
            for (;;) {
                if (hasRight(i)) {
                    if (_mem[left(i)].weight < _mem[right(i)].weight) {
                        if (_mem[left(i)].weight < _mem[i].weight) {
                            temp = _mem[i];
                            _mem[i] = _mem[left(i)];
                            _mem[left(i)] = temp;
                            i = left(i);
                        } else break;
                    } else {
                        if (_mem[right(i)].weight < _mem[i].weight) {
                            temp = _mem[i];
                            _mem[i] = _mem[right(i)];
                            _mem[right(i)] = temp;
                            i = right(i);
                        } else break;
                    }
                } else if (hasLeft(i)) {
                    if (_mem[left(i)].weight < _mem[i].weight) {
                        temp = _mem[i];
                        _mem[i] = _mem[left(i)];
                        _mem[left(i)] = temp;
                        i = left(i);
                    } else break;
                } else break;
            }
        }
        
        int parent(const int i) const 
        {
            return (i-1)/2;
        }
        
        int left(const int i) const 
        {
            return i*2+1;
        }
        
        int right(const int i) const 
        {
            return i*2+2;
        }
        
        bool hasLeft(const int i) const 
        {
            return left(i) < _eltos;
        }
        
        bool hasRight(const int i) const 
        {
            return right(i) < _eltos;
        }
        
    private:
        
        int _k;
        
        int _eltos;
        
        Node *_mem;
        
};

#endif // TOPK_H

