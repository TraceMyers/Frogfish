#pragma once

#include <BWEM/bwem.h>
#include <string.h>

#define BWEMBARRAY_INIT_SIZE 30
#define BWEMBARRAY_RESIZE_CONST 20

class BWEMBaseArray {

private:

    const BWEM::Base **array = new const BWEM::Base *[BWEMBARRAY_INIT_SIZE];
    int size = BWEMBARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        const BWEM::Base **temp = new const BWEM::Base *[size + BWEMBARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(const BWEM::Base *) * size);
        delete array;
        array = temp;
        size += BWEMBARRAY_RESIZE_CONST;
    }

public:

    void add(const BWEM::Base *bwem_base) {
        assert(bwem_base != nullptr);
        array[len] = bwem_base;
        len++;
        if (len == size) {
            resize();
        }
    }

    const BWEM::Base *remove(const BWEM::Base *bwem_base) {
        assert(bwem_base != nullptr);
        const BWEM::Base *rm_bwem_base = nullptr;
        for (register int i = 0; i < len; i++) {
            if (array[i]->GetArea()->Id() == bwem_base->GetArea()->Id()) {
                rm_bwem_base = array[i];
                array[i] = array[len - 1];
                len--;
            } 
        }
        return rm_bwem_base;
    }

    const BWEM::Base *remove_at(int i) {
        const BWEM::Base *rm_bwem_base = nullptr;
        if (i < len && len > 0) {
            rm_bwem_base = array[i];
            array[i] = array[len - 1];
            len--;
        }
        return rm_bwem_base;
    }

    int length() const {return len;}

    const BWEM::Base *operator [] (int i) const {return array[i];}

    int find(const BWEM::Base *bwem_base) const {
        assert(bwem_base != nullptr);
        for (register int i = 0; i < len; i++) {
            if (array[i]->GetArea()->Id() == bwem_base->GetArea()->Id()) {
                return i;
            }    
        }
        return -1;
    }

    void clear() {len = 0;}

    // only for use at end of program
    void free_data() {delete array;}
};