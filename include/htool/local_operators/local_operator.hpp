
#ifndef HTOOL_TESTING_LOCAL_OPERATOR_HPP
#define HTOOL_TESTING_LOCAL_OPERATOR_HPP

#include "../basic_types/matrix.hpp"
#include "../clustering/cluster_node.hpp"
#include "../hmatrix/interfaces/virtual_generator.hpp"
#include "../local_operators/virtual_local_operator.hpp"

namespace htool {

template <typename CoefficientPrecision, typename CoordinatePrecision = CoefficientPrecision>
class LocalOperator : public VirtualLocalOperator<CoefficientPrecision> {
  protected:
    std::shared_ptr<const Cluster<CoordinatePrecision>> m_target_root_cluster, m_source_root_cluster;

    char m_symmetry{'N'};
    char m_UPLO{'N'};

    bool m_target_use_permutation_to_mvprod{false}; // Permutation used when add_mvprod, useful for offdiag
    bool m_source_use_permutation_to_mvprod{false}; // Permutation used when add_mvprod, useful for offdiag

    LocalOperator(std::shared_ptr<const Cluster<CoordinatePrecision>> cluster_tree_target, std::shared_ptr<const Cluster<CoordinatePrecision>> cluster_tree_source, char symmetry = 'N', char UPLO = 'N', bool target_use_permutation_to_mvprod = false, bool source_use_permutation_to_mvprod = false) : m_target_root_cluster(cluster_tree_target), m_source_root_cluster(cluster_tree_source), m_symmetry(symmetry), m_UPLO(UPLO), m_target_use_permutation_to_mvprod(target_use_permutation_to_mvprod), m_source_use_permutation_to_mvprod(source_use_permutation_to_mvprod) {}

    virtual void local_add_vector_product(char trans, CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out) const                                                       = 0;
    virtual void local_add_vector_product_symmetric(char trans, CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out, char UPLO, char symmetry) const                   = 0;
    virtual void local_add_matrix_product_row_major(char trans, CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out, int mu) const                                     = 0;
    virtual void local_add_matrix_product_symmetric_row_major(char trans, CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out, int mu, char UPLO, char symmetry) const = 0;

  public:
    // -- Operations --
    void add_vector_product_global_to_local(CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out) const override {

        // Permutation
        std::vector<CoefficientPrecision> buffer_in(m_source_use_permutation_to_mvprod ? m_source_root_cluster->get_size() : 0);
        std::vector<CoefficientPrecision> buffer_out(m_target_use_permutation_to_mvprod ? m_target_root_cluster->get_size() : 0);

        // int rankWorld;
        // MPI_Comm_rank(MPI_COMM_WORLD, &rankWorld);
        // if (rankWorld == 0) {
        //     std::cout << m_cluster_tree_source->get_permutation() << "\n";
        //     std::cout << "test in  " << m_source_root_cluster->get_offset() << " " << m_source_root_cluster->get_size() << "\n";
        //     for (int i = 0; i < m_source_root_cluster->get_size(); i++) {
        //         std::cout << in[i + m_source_root_cluster->get_offset()] << ",";
        //     }
        //     std::cout << "\n";
        // }

        if (m_source_use_permutation_to_mvprod) {
            global_to_root_cluster(*m_source_root_cluster, in + m_source_root_cluster->get_offset(), buffer_in.data());
        }
        if (m_target_use_permutation_to_mvprod) {
            global_to_root_cluster(*m_target_root_cluster, out, buffer_out.data());
        }

        const CoefficientPrecision *input = m_source_use_permutation_to_mvprod ? buffer_in.data() : in + m_source_root_cluster->get_offset();
        CoefficientPrecision *output      = m_target_use_permutation_to_mvprod ? buffer_out.data() : out;

        // MPI_Comm_rank(MPI_COMM_WORLD, &rankWorld);
        // if (rankWorld == 0) {
        //     std::cout << "test " << m_source_root_cluster->get_size() << "\n";
        //     for (int i = 0; i < m_source_root_cluster->get_size(); i++) {
        //         std::cout << input[i] << ",";
        //     }
        //     std::cout << "\n";
        // }

        // Local to local product
        if (m_symmetry == 'N') {
            local_add_vector_product('N', alpha, input, beta, output);
        } else if (m_symmetry == 'S' || m_symmetry == 'H') {
            local_add_vector_product_symmetric('N', alpha, input, beta, output, m_UPLO, m_symmetry);
        } else {
            throw std::invalid_argument("[Htool error] Invalid arguments for LocalDenseMatrix");
        }

        // int rankWorld;
        // MPI_Comm_rank(MPI_COMM_WORLD, &rankWorld);
        // if (rankWorld == 0) {
        //     std::cout << "test " << m_target_root_cluster->get_size() << "\n";
        //     for (int i = 0; i < m_target_root_cluster->get_size(); i++) {
        //         std::cout << i << " " << out[i] << ",";
        //     }
        //     std::cout << "\n";
        // }

        // Permutation
        if (m_target_use_permutation_to_mvprod) {
            root_cluster_to_global(*m_target_root_cluster, output, out);
        }
    };
    void add_matrix_product_global_to_local(CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out, int mu) const override {
        //
        int nc = m_source_root_cluster->get_size();
        int nr = m_target_root_cluster->get_size();
        std::vector<CoefficientPrecision> in_perm(m_source_use_permutation_to_mvprod || m_symmetry == 'H' ? nc * mu : 0);
        std::vector<CoefficientPrecision> buffer_in(m_source_use_permutation_to_mvprod ? nc * mu + nc : 0);
        std::vector<CoefficientPrecision> buffer_out(m_target_use_permutation_to_mvprod ? nr * mu : 0);

        CoefficientPrecision *output = m_target_use_permutation_to_mvprod ? buffer_out.data() : out;
        // Permutation + transpose
        if (m_source_use_permutation_to_mvprod) {
            for (int i = 0; i < mu; i++) {
                for (int j = 0; j < nc; j++) {
                    buffer_in[j] = in[i + (m_source_root_cluster->get_offset() + j) * mu];
                }
                global_to_root_cluster(*m_source_root_cluster, buffer_in.data(), buffer_in.data() + nc + nc * i);
            }
            for (int i = 0; i < mu; i++) {
                for (int j = 0; j < nc; j++) {
                    in_perm[i + j * mu] = buffer_in[nc + j + i * nc];
                }
            }
            if (m_symmetry == 'H') {
                // conj_if_complex<CoefficientPrecision>(in_perm.data(), in_perm.size());
            }
        } else if (m_symmetry == 'H') { // we have to conjugate to apply in this particular case
            std::copy_n(in + m_source_root_cluster->get_offset() * mu, in_perm.size(), in_perm.data());
            // conj_if_complex<CoefficientPrecision>(in_perm.data(), in_perm.size());
        }

        const CoefficientPrecision *input = m_source_use_permutation_to_mvprod || m_symmetry == 'H' ? in_perm.data() : in + m_source_root_cluster->get_offset() * mu;

        // Local to local product
        if (m_symmetry == 'N') {
            local_add_matrix_product_row_major('N', alpha, input, beta, output, mu);
        } else if (m_symmetry == 'S' || m_symmetry == 'H') {
            local_add_matrix_product_symmetric_row_major('N', alpha, input, beta, output, mu, m_UPLO, m_symmetry);
        } else {
            throw std::invalid_argument("[Htool error] Invalid arguments for LocalDenseMatrix");
        }

        // if (m_symmetry == 'H') {
        //     conj_if_complex<CoefficientPrecision>(output, nr * mu);
        // }

        // Permutation TODO
        // if (m_target_use_permutation_to_mvprod) {
        //     for (int i = 0; i < mu; i++) {
        //         root_cluster_to_global(m_cluster_tree_target.get(), output + i * nr, out + i * nr);
        //     }
        // }
    }

    void add_vector_product_transp_local_to_global(CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out) const override {

        std::vector<CoefficientPrecision> buffer_out(m_source_use_permutation_to_mvprod ? m_source_root_cluster->get_size() : 0);
        CoefficientPrecision *output = m_source_use_permutation_to_mvprod ? buffer_out.data() : out + m_source_root_cluster->get_offset();

        if (m_source_use_permutation_to_mvprod) {
            global_to_root_cluster(*m_source_root_cluster, out + m_source_root_cluster->get_offset(), buffer_out.data());
        }

        // Local to local product
        if (m_symmetry == 'N') {
            local_add_vector_product('T', alpha, in, beta, output);
        } else if (m_symmetry == 'S' || m_symmetry == 'H') {
            // m_data.add_vector_product('T', alpha, in, beta, output);
            local_add_vector_product_symmetric('T', alpha, in, beta, output, m_UPLO, m_symmetry);
        } else {
            throw std::invalid_argument("[Htool error] Invalid arguments for LocalDenseMatrix");
        }

        // Permutation
        if (m_source_use_permutation_to_mvprod) {
            root_cluster_to_global(*m_source_root_cluster, output, out + m_source_root_cluster->get_offset());
        }
    }

    void add_matrix_product_transp_local_to_global(CoefficientPrecision alpha, const CoefficientPrecision *in, CoefficientPrecision beta, CoefficientPrecision *out, int mu) const override {

        int nc = m_source_root_cluster->get_size();
        int nr = m_target_root_cluster->get_size();
        std::vector<CoefficientPrecision> input_perm(m_symmetry == 'H' ? nr * mu : 0);
        std::vector<CoefficientPrecision> buffer_out(m_source_use_permutation_to_mvprod ? nc * mu + nc : 0);
        std::vector<CoefficientPrecision> output_perm(nc * mu);

        if (m_symmetry == 'H') { // we have to conjugate to apply in this particular case
            std::copy_n(in, input_perm.size(), input_perm.data());
            conj_if_complex<CoefficientPrecision>(input_perm.data(), input_perm.size());
        }

        const CoefficientPrecision *input = m_symmetry == 'H' ? input_perm.data() : in;

        // Local to local product
        if (m_symmetry == 'N') {
            local_add_matrix_product_row_major('T', alpha, input, beta, output_perm.data(), mu);
        } else if (m_symmetry == 'S' || m_symmetry == 'H') {
            local_add_matrix_product_symmetric_row_major('T', alpha, input, beta, output_perm.data(), mu, m_UPLO, m_symmetry);
        } else {
            throw std::invalid_argument("[Htool error] Invalid arguments for LocalDenseMatrix");
        }

        // Permutation + transpose
        if (m_source_use_permutation_to_mvprod) {
            for (int i = 0; i < mu; i++) {
                for (int j = 0; j < nc; j++) {
                    buffer_out[j] = output_perm[i + j * mu];
                }
                root_cluster_to_global(*m_source_root_cluster, buffer_out.data(), buffer_out.data() + nc + nc * i);
            }
            for (int i = 0; i < mu; i++) {
                for (int j = 0; j < nc; j++) {
                    out[i + (m_source_root_cluster->get_offset() + j) * mu] = buffer_out[nc + j + i * nc];
                }
            }
        } else { // we have to conjugate to apply in this particular case
            std::copy_n(output_perm.data(), output_perm.size(), out + m_source_root_cluster->get_offset() * mu);
        }
        if (m_symmetry == 'H') {
            conj_if_complex<CoefficientPrecision>(out + m_source_root_cluster->get_offset() * mu, nc * mu);
        }
    }
};
} // namespace htool
#endif