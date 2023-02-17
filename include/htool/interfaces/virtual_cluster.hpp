#ifndef HTOOL_CLUSTERING_VIRTUAL_CLUSTER_HPP
#define HTOOL_CLUSTERING_VIRTUAL_CLUSTER_HPP

#include "../basic_types/matrix.hpp"
#include "../misc/user.hpp"
#include <functional>
#include <memory>
#include <mpi.h>
#include <stack>

namespace htool {

// template <typename T>
class VirtualCluster {
  public:
    // build without partition
    virtual void build(int nb_pt0, const double *const x0, const double *const r0, const double *const g0, int nb_sons = -1) = 0;

    // build without partition
    virtual void build(int nb_pt0, const double *const x0, int nb_sons = -1) = 0;

    // build with partition
    virtual void build(int nb_pt0, const double *const x0, const double *const r0, const double *const g0, const int *const MasterOffset0, int nb_sons = -1) = 0;

    // build with partition
    virtual void build(int nb_pt0, const double *const x0, const int *const MasterOffset0, int nb_sons = -1) = 0;

    //// Getters for current cluster
    virtual double get_rad() const                                   = 0;
    virtual const std::vector<double> &get_ctr() const               = 0;
    virtual const VirtualCluster &get_son(const int &j) const        = 0;
    virtual VirtualCluster &get_son(const int &j)                    = 0;
    virtual int get_depth() const                                    = 0;
    virtual int get_rank() const                                     = 0;
    virtual int get_offset() const                                   = 0;
    virtual int get_size() const                                     = 0;
    virtual const int *get_perm_data() const                         = 0;
    virtual int *get_perm_data()                                     = 0;
    virtual int get_nb_sons() const                                  = 0;
    virtual int get_counter() const                                  = 0;
    virtual bool is_leaf() const                                     = 0;
    virtual std::shared_ptr<VirtualCluster> get_cluster_tree() const = 0;

    // Getters for local cluster
    virtual const VirtualCluster &get_local_cluster() const           = 0;
    virtual std::shared_ptr<VirtualCluster> get_local_cluster_tree()  = 0;
    virtual std::vector<int> get_local_perm() const                   = 0;
    virtual int get_local_offset() const                              = 0;
    virtual int get_local_size() const                                = 0;
    virtual std::pair<int, int> get_masteroffset_on_rank(int i) const = 0;
    virtual std::pair<int, int> get_local_masteroffset() const        = 0;
    virtual bool is_local() const                                     = 0;

    //// Getters for global data
    virtual int get_space_dim() const                                        = 0;
    virtual int get_minclustersize() const                                   = 0;
    virtual int get_max_depth() const                                        = 0;
    virtual int get_min_depth() const                                        = 0;
    virtual const std::vector<int> &get_global_perm() const                  = 0;
    virtual int get_global_perm(int i) const                                 = 0;
    virtual const int *get_global_perm_data() const                          = 0;
    virtual int *get_global_perm_data()                                      = 0;
    virtual const VirtualCluster *get_root() const                           = 0;
    virtual const std::vector<std::pair<int, int>> &get_masteroffset() const = 0;

    //// Setters
    virtual void set_rank(int rank0)                              = 0;
    virtual void set_offset(int offset0)                          = 0;
    virtual void set_size(int size0)                              = 0;
    virtual void set_minclustersize(unsigned int minclustersize0) = 0;

    // Output
    virtual void print() const                                                                                     = 0;
    virtual void save_geometry(const double *const x0, std::string filename, const std::vector<int> &depths) const = 0;
    virtual void save_cluster(std::string filename) const                                                          = 0;
    virtual void read_cluster(std::string file_permutation, std::string file_tree)                                 = 0;

    virtual ~VirtualCluster(){};
};

// Permutations
template <typename T>
void cluster_to_global(const VirtualCluster *const cluster_tree, const T *const in, T *const out) {
    for (int i = 0; i < cluster_tree->get_size(); i++) {
        out[cluster_tree->get_global_perm(i)] = in[i];
    }
}
template <typename T>
void global_to_cluster(const VirtualCluster *const cluster_tree, const T *const in, T *const out) {
    for (int i = 0; i < cluster_tree->get_size(); i++) {
        out[i] = in[cluster_tree->get_global_perm(i)];
    }
}

// Local permutations
template <typename T>
void local_cluster_to_local(const VirtualCluster *const cluster_tree, const T *const in, T *const out) {
    if (!cluster_tree->is_local()) {
        throw std::logic_error("[Htool error] Permutation is not local, local_cluster_to_local cannot be used"); // LCOV_EXCL_LINE
    } else {
        for (int i = 0; i < cluster_tree->get_local_masteroffset().second; i++) {
            out[cluster_tree->get_global_perm(cluster_tree->get_local_masteroffset().first + i) - cluster_tree->get_local_masteroffset().first] = in[i];
        }
    }
}
template <typename T>
void local_to_local_cluster(const VirtualCluster *const cluster_tree, const T *const in, T *const out) {
    if (!cluster_tree->is_local()) {
        throw std::logic_error("[Htool error] Permutation is not local, local_to_local_cluster cannot be used"); // LCOV_EXCL_LINE
    } else {
        for (int i = 0; i < cluster_tree->get_local_masteroffset().second; i++) {
            out[i] = in[cluster_tree->get_global_perm(cluster_tree->get_local_masteroffset().first + i) - cluster_tree->get_local_masteroffset().first];
        }
    }
}

} // namespace htool
#endif