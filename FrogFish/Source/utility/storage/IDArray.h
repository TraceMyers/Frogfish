#ifndef ID_ARRAY
#define ID_ARRAY

#include <string.h>

#define IDARRAY_INIT_LEN 800
#define IDARRAY_RESIZE_CONST 100

class IDArray {

private:

    int *array = new int[IDARRAY_INIT_LEN];
    int size = IDARRAY_INIT_LEN;
    int len = 0;

    void resize() {
        int *temp = new int[size + IDARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(int) * size);
        array = temp;
        size += IDARRAY_RESIZE_CONST;
    }

public:

    void add(int f_unit) {
        array[len] = f_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    void remove(int ID) {
        const register int last_val_i = len - 1;
        for (register int i = 0; i < len; i++) {
            if (array[i] == ID) {
                if (i != last_val_i) {
                    array[i] = array[last_val_i];
                }
                if (len > 0) {
                    len--;
                }
                break;
            }    
        }
    }

    void clear() {
        len = 0;
    }

    bool has(int ID) {
        const register int last_val_i = len - 1;
        for (register int i = 0; i < len; i++) {
            if (array[i] == ID) {
                return true;
            }    
        }
        return false;
    }

    void free_data() {
        // only for use at end of program
        // called by owner
        delete array;
    }
};

#endif