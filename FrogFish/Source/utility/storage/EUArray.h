#ifndef EU_ARRAY
#define EU_ARRAY

#include "../data/EnemyUnit.h"
#include <string.h>

#define EUARRAY_INIT_LEN 50
#define EUARRAY_RESIZE_CONST 25

class EUArray {

private:

    EUnit *array = new EUnit[EUARRAY_INIT_LEN];
    int size = EUARRAY_INIT_LEN;
    int len = 0;

    void resize() {
        EUnit *temp = new EUnit[size + EUARRAY_RESIZE_CONST];
        memcpy(temp, array, sizeof(EUnit) * size);
        array = temp;
        size += EUARRAY_RESIZE_CONST;
    }

public:

    void add(EUnit e_unit) {
        array[len] = e_unit;
        len++;
        if (len == size) {
            resize();
        }
    }

    void remove(int ID) {
        const register int last_val_i = len - 1;
        for (register int i = 0; i < len; i++) {
            if ((array[i])->get_ID() == ID) {
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

    int length() {
        return len;
    }

    EUnit operator [](int i) {
        return array[i];
    }

    EUnit find(int ID) {
        const register int last_val_i = len - 1;
        for (register int i = 0; i < len; i++) {
            if ((array[i])->get_ID() == ID) {
                return array[i];
            }    
        }
        return nullptr;
    }

    void free_data() {
        // only for use at end of program
        delete array;
    }
};

#endif