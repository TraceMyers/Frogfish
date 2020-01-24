#pragma once
#pragma message("including BWEMBaseArray")

#include <BWEM/bwem.h>
#include <string.h>


class BWEMBaseArray {

public:

    void add(const BWEM::Base *bwem_base) {
        assert(bwem_base != nullptr);
        array[_size] = bwem_base;
        _size++;
        if (_size == internal_size) {
            resize();
        }
    }

    const BWEM::Base *remove(const BWEM::Base *bwem_base) {
        assert(bwem_base != nullptr);
        const BWEM::Base *rm_bwem_base = nullptr;
        for (register int i = 0; i < _size; i++) {
            if (array[i]->GetArea()->Id() == bwem_base->GetArea()->Id()) {
                rm_bwem_base = array[i];
                array[i] = array[_size - 1];
                _size--;
            } 
        }
        return rm_bwem_base;
    }

    const BWEM::Base *remove_at(int i) {
        const BWEM::Base *rm_bwem_base = nullptr;
        if (i < _size && _size > 0) {
            rm_bwem_base = array[i];
            array[i] = array[_size - 1];
            _size--;
        }
        return rm_bwem_base;
    }

    int size() const {return _size;}

    const BWEM::Base *operator [] (int i) const {return array[i];}

    int find(const BWEM::Base *bwem_base) const {
        assert(bwem_base != nullptr);
        for (register int i = 0; i < _size; i++) {
            if (array[i]->GetArea()->Id() == bwem_base->GetArea()->Id()) {
                return i;
            }    
        }
        return -1;
    }

    void clear() {_size = 0;}

    void free_data() {delete array;}

private:

    const int BWEMBARRAY_INIT_SIZE = 30;
    const int BWEMBARRAY_RESIZE_CONST = 10;

    const BWEM::Base **array = new const BWEM::Base *[BWEMBARRAY_INIT_SIZE];
    int internal_size = BWEMBARRAY_INIT_SIZE;
    int _size = 0;

    void resize() {
        const BWEM::Base **temp 
            = new const BWEM::Base *[internal_size + BWEMBARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(const BWEM::Base *) * internal_size);
        delete array;
        array = temp;
        internal_size += BWEMBARRAY_RESIZE_CONST;
    }

};