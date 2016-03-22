#ifndef PALLOCABLE_H
#define PALLOCABLE_H

#include "postgres.h"       //standard
#include "dfg_utils.h"

class PAllocable {
    
    public:
        
        void* operator new(size_t s)
        {
            void *ptr;
            PROTECTED_CONTEXT( ptr = palloc( s ) );
            return ptr;
        }
        
        void* operator new[](size_t s)
        {
            void *ptr;
            PROTECTED_CONTEXT( ptr = palloc( s ) );
            return ptr;
        }
        
        void operator delete(void* x)
        {
            
        }
        
        void operator delete[](void* x)
        {
            
        }
        
};

#endif // PALLOCABLE_H

