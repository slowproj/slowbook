// slowbook/dummydata.hpp //
// Created by Sanshiro Enomoto on 5 March 2026

#pragma once

#include <unistd.h>
#include <time.h>
#include <cmath>
#include <random>


namespace slowbook {

static std::random_device random_device;
static std::mt19937 random_generator(random_device());


class RandomWalk {
  public:
    RandomWalk(double step=1): step_dist{0, step} {
        x = 0;
    }
    double get() {
        x += step_dist(random_generator);
        return x;
    }
  private:
    std::normal_distribution<> step_dist;
  private:
    double x;
};

    

class RandomPulse {
  public:
    RandomPulse(double rate=10, double n = 1, double m=5): mu{pow(m/2, 0.1)}, n_dist{n}, q_dist{mu}, t_dist{rate} {}
    double get() {
        double dt = t_dist(random_generator);
        usleep(int(dt*1e6));
        
        int n; do {
            n = n_dist(random_generator);
        } while (n == 0);
        
        double q = 0;
        for (int i = 0; i < n; i++) {
            double qk = 0;
            for (int k = 0; k < 10; k++) {
                qk = mu * qk + q_dist(random_generator);
            }
            q += qk;
        }
        return q;
    }
  private:
    double mu;
    std::exponential_distribution<> t_dist;
    std::poisson_distribution<> n_dist, q_dist;
};


}
