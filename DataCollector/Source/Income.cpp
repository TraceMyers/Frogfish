#include "Income.h"
#include <vector>
#include <math.h>

namespace Income {

    namespace {

        // 24 * 12 (choosing 24 fps somewhat arbitrarily, so it's 12 seconds)
        static const int BUFF_FRAMES = 288;

        class CircleBuff {
        public:

            void push(int val) {
                buff[cur_frame++] = val;
                if (cur_frame == BUFF_FRAMES) {
                    completed_cycle = true;
                    cur_frame = 0;
                }
            }

            // shouldn't use until buff full
            const std::vector<int> &get_oldest_to_newest() {
                ordered_frames.clear();
                int start = cur_frame,
                    i     = cur_frame;
                while (true) {
                    ordered_frames.push_back(buff[i++]);
                    if (i == BUFF_FRAMES) {
                        i = 0;
                    }
                    if (i == start) {
                        break;
                    }
                }
                return ordered_frames;
            }

            bool full() {
                return completed_cycle;
            }

        private:

            int cur_frame = 0;
            bool completed_cycle = false;
            int buff[BUFF_FRAMES] {0};
            std::vector<int> ordered_frames;

        };

        CircleBuff  mineral_buffs[2];
        CircleBuff  gas_buffs[2];
        int         gathered_minerals_sums[2] {0, 0};
        int         gathered_gas_sums[2] {0, 0};
        double      weights[BUFF_FRAMES];
        double      mps[2];
        double      gps[2];

        // Uses approximate beta probability distribution function
        void set_weights(double a=1, double b=0.8) {
            double sum = 0;
            double factor = 1/(BUFF_FRAMES + 1);
            double alpha_exponent = a - 1;
            double beta_exponent = b - 1;
            for (int i = 1; i < BUFF_FRAMES + 1; ++i) {
                double val_1 = i * factor;
                double val_2 = 1 - val_1;
                double weight = pow(val_1, alpha_exponent) * pow(val_2, beta_exponent);
                weights[i - 1] = weight;
                sum += weight;
            }
            for (int i = 0; i <BUFF_FRAMES; ++i) {
                weights[i] /= sum;
            }
        }
    }

    void init() {
        set_weights();
    }

    void on_frame_update(const BWAPI::Player *players) {
        for (int i = 0; i < 2; ++i) {
            int gathered_minerals = (*(players + i))->gatheredMinerals();
            int min_diff = gathered_minerals - gathered_minerals_sums[i];
            mineral_buffs[i].push(min_diff);
            gathered_minerals_sums[i] = gathered_minerals;
            int gathered_gas = (*(players + i))->gatheredGas();
            int gas_diff = gathered_gas - gathered_gas_sums[i];
            gas_buffs[i].push(gas_diff);
            gathered_gas_sums[i] = gathered_gas;
        }
    }

    bool ready() {
        return mineral_buffs[0].full();
    }

    double *get_mps() {
        for (int i = 0; i < 2; ++i) {
            mps[i] = 0.0;
            const auto &ordered = mineral_buffs[i].get_oldest_to_newest();
            for (int j = 0; j < BUFF_FRAMES; ++j) {
                mps[i] += ordered[j] * weights[j];
            }
        }
        return mps;
    }

    double *get_gps() {
        for (int i = 0; i < 2; ++i) {
            gps[i] = 0.0;
            const auto &ordered = gas_buffs[i].get_oldest_to_newest();
            for (int j = 0; j < BUFF_FRAMES; ++j) {
                gps[i] += ordered[j] * weights[j];
            }
        }
        return gps;
    }
}
