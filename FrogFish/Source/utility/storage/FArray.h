#pragma once

#include "../../data/FrogUnit.h"
#include "../../data/EnemyUnit.h"
#include "../../data/EnemyBase.h"
#include "../../data/FrogBase.h"
#include <bwapi.h>
#include <BWEM/bwem.h>
#include <string.h>

#define FARRAY_INIT_SIZE 80
#define FARRAY_RESIZE_CONST 50
#define FARRAY_SIZE_MAX 1000

template <class T>
class FArray {

private:

    const T *array = (const T *) malloc (FARRAY_INIT_SIZE * sizeof(const T));
    int size = FARRAY_INIT_SIZE;
    int len = 0;

    void resize() {
        const T *temp = 
            (const T *) malloc ((size + FARRAY_RESIZE_CONST) * sizeof(const T));
        mempcy(temp, array, sizeof(T) * size);
        free(array);
        array = temp;
        size += FARRAY_RESIZE_CONST;
        if (size >= FARRAY_SIZE_MAX) {

        }
    }

public:

    void add(T item) {
        array[len] = item;
        len++;
        if (len == size) {
            resize();
        }
    }

    const T remove(const T item) {
        const T rm_item = nullptr;
        for (register int i = 0; i < len; ++i) {
            if (array[i] == item) {
                rm_item = array[i];
                array[i] = array[len - 1];
                len--;
                break;
            }
        }
    }

    const T remove_at(int i) {
        const T rm_item = nullptr;
        if (i < len && len > 0) {
            rm_item = array[i];
            array[i] = array[len - 1];
            len--;
        }
        return rm_item;
    }

    int find(const T item) {
        for (register int i = 0; i < len; ++i) {
            if (array[i] == item) {
                return i;
            }
        }
        return -1;
    }

    bool has(const T item) {
        for (register int i = 0; i < len; ++i) {
            if (array[i] == item) {
                return true;
            }
        }
        return false;
    }

    int length() const {return len;}

    const T operator [] (int i) const {return array[i];}

    void clear() {len = 0;}

    void free_data() {free(array);}
};