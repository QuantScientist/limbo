//| Copyright Inria May 2015
//| This project has received funding from the European Research Council (ERC) under
//| the European Union's Horizon 2020 research and innovation programme (grant
//| agreement No 637972) - see http://www.resibots.eu
//|
//| Contributor(s):
//|   - Jean-Baptiste Mouret (jean-baptiste.mouret@inria.fr)
//|   - Antoine Cully (antoinecully@gmail.com)
//|   - Kontantinos Chatzilygeroudis (konstantinos.chatzilygeroudis@inria.fr)
//|   - Federico Allocati (fede.allocati@gmail.com)
//|   - Vaios Papaspyros (b.papaspyros@gmail.com)
//|   - Roberto Rama (bertoski@gmail.com)
//|
//| This software is a computer library whose purpose is to optimize continuous,
//| black-box functions. It mainly implements Gaussian processes and Bayesian
//| optimization.
//| Main repository: http://github.com/resibots/limbo
//| Documentation: http://www.resibots.eu/limbo
//|
//| This software is governed by the CeCILL-C license under French law and
//| abiding by the rules of distribution of free software.  You can  use,
//| modify and/ or redistribute the software under the terms of the CeCILL-C
//| license as circulated by CEA, CNRS and INRIA at the following URL
//| "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and  rights to copy,
//| modify and redistribute granted by the license, users are provided only
//| with a limited warranty  and the software's author,  the holder of the
//| economic rights,  and the successive licensors  have only  limited
//| liability.
//|
//| In this respect, the user's attention is drawn to the risks associated
//| with loading,  using,  modifying and/or developing or reproducing the
//| software by the user in light of its specific status of free software,
//| that may mean  that it is complicated to manipulate,  and  that  also
//| therefore means  that it is reserved for developers  and  experienced
//| professionals having in-depth computer knowledge. Users are therefore
//| encouraged to load and test the software's suitability as regards their
//| requirements in conditions enabling the security of their systems and/or
//| data to be ensured and,  more generally, to use and operate it in the
//| same conditions as regards security.
//|
//| The fact that you are presently reading this means that you have had
//| knowledge of the CeCILL-C license and that you accept its terms.
//|
#ifndef LIMBO_OPT_NLOPT_NO_GRAD_HPP
#define LIMBO_OPT_NLOPT_NO_GRAD_HPP

#ifndef USE_NLOPT
#warning No NLOpt
#else
#include <Eigen/Core>

#include <vector>

#include <nlopt.hpp>

#include <limbo/tools/macros.hpp>
#include <limbo/opt/optimizer.hpp>

namespace limbo {
    namespace defaults {
        struct opt_nloptnograd {
            /// @ingroup opt_defaults
            /// number of calls to the optimized function
            BO_PARAM(int, iterations, 500);
        };
    }
    namespace opt {
        /**
          @ingroup opt
        Binding to gradient-free NLOpt algorithms.
         See: http://ab-initio.mit.edu/wiki/index.php/NLopt_Algorithms

         Algorithms:
         - GN_DIRECT
         - GN_DIRECT_L, [default]
         - GN_DIRECT_L_RAND
         - GN_DIRECT_NOSCAL
         - GN_DIRECT_L_NOSCAL
         - GN_DIRECT_L_RAND_NOSCAL
         - GN_ORIG_DIRECT
         - GN_ORIG_DIRECT_L
         - GN_CRS2_LM
         - GN_MLSL
         - GN_MLSL_LDS
         - GN_ISRES
         - LN_COBYLA
         - LN_AUGLAG_EQ
         - LN_BOBYQA
         - LN_NEWUOA
         - LN_NEWUOA_BOUND
         - LN_NELDERMEAD
         - LN_SBPLX
         - LN_AUGLAG

         Parameters:
         - int iterations
        */
        template <typename Params, nlopt::algorithm Algorithm = nlopt::GN_DIRECT_L_RAND>
        struct NLOptNoGrad {
        public:
            template <typename F>
            Eigen::VectorXd operator()(const F& f, const Eigen::VectorXd& init, bool bounded) const
            {
                // Assert that the algorithm is non-gradient
                // TO-DO: Add support for MLSL (Multi-Level Single-Linkage)
                // TO-DO: Add better support for ISRES (Improved Stochastic Ranking Evolution Strategy)
                // clang-format off
                static_assert(Algorithm == nlopt::LN_COBYLA || Algorithm == nlopt::LN_BOBYQA ||
                    Algorithm == nlopt::LN_NEWUOA || Algorithm == nlopt::LN_NEWUOA_BOUND ||
                    Algorithm == nlopt::LN_PRAXIS || Algorithm == nlopt::LN_NELDERMEAD ||
                    Algorithm == nlopt::LN_SBPLX || Algorithm == nlopt::GN_DIRECT ||
                    Algorithm == nlopt::GN_DIRECT_L || Algorithm == nlopt::GN_DIRECT_L_RAND ||
                    Algorithm == nlopt::GN_DIRECT_NOSCAL || Algorithm == nlopt::GN_DIRECT_L_NOSCAL ||
                    Algorithm == nlopt::GN_DIRECT_L_RAND_NOSCAL || Algorithm == nlopt::GN_ORIG_DIRECT ||
                    Algorithm == nlopt::GN_ORIG_DIRECT_L || Algorithm == nlopt::GN_CRS2_LM ||
                    Algorithm == nlopt::GD_STOGO || Algorithm == nlopt::GD_STOGO_RAND ||
                    Algorithm == nlopt::GN_ISRES || Algorithm == nlopt::GN_ESCH, "NLOptNoGrad accepts gradient free nlopt algorithms only");
                // clang-format on

                int dim = init.size();
                nlopt::opt opt(Algorithm, dim);

                opt.set_max_objective(nlopt_func<F>, (void*)&f);

                std::vector<double> x(dim);
                Eigen::VectorXd::Map(&x[0], dim) = init;

                opt.set_maxeval(Params::opt_nloptnograd::iterations());

                if (bounded) {
                    opt.set_lower_bounds(std::vector<double>(dim, 0));
                    opt.set_upper_bounds(std::vector<double>(dim, 1));
                }

                double max;

                try {
                    opt.optimize(x, max);
                }
                catch (nlopt::roundoff_limited& e) {
                    // In theory it's ok to ignore this error
                    std::cerr << "[NLOptNoGrad]: " << e.what() << std::endl;
                }
                catch (std::invalid_argument& e) {
                    // In theory it's ok to ignore this error
                    std::cerr << "[NLOptNoGrad]: " << e.what() << std::endl;
                }
                catch (std::runtime_error& e) {
                    // In theory it's ok to ignore this error
                    std::cerr << "[NLOptGrad]: " << e.what() << std::endl;
                }

                return Eigen::VectorXd::Map(x.data(), x.size());
            }

        protected:
            template <typename F>
            static double nlopt_func(const std::vector<double>& x, std::vector<double>& grad, void* my_func_data)
            {
                F* f = (F*)(my_func_data);
                Eigen::VectorXd params = Eigen::VectorXd::Map(x.data(), x.size());
                double v = eval(*f, params);
                return v;
            }
        };
    }
}

#endif
#endif
