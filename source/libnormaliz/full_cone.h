/*
 * Normaliz
 * Copyright (C) 2007-2019  Winfried Bruns, Bogdan Ichim, Christof Soeger
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As an exception, when this program is distributed through (i) the App Store
 * by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or (iii) Google Play
 * by Google Inc., then that store may impose any digital rights management,
 * device limits and/or redistribution restrictions that are required by its
 * terms of service.
 */

#ifndef LIBNORMALIZ_FULL_CONE_H
#define LIBNORMALIZ_FULL_CONE_H

#include <list>
#include <vector>
#include <deque>
//#include <set>
#include <boost/dynamic_bitset.hpp>

#include "libnormaliz/general.h"
#include "libnormaliz/cone.h"
//#include "libnormaliz/cone_property.h"
#include "libnormaliz/matrix.h"
#include "libnormaliz/simplex.h"
#include "libnormaliz/cone_dual_mode.h"
#include "libnormaliz/HilbertSeries.h"
#include "libnormaliz/reduction.h"
// #include "libnormaliz/sublattice_representation.h"
#include "libnormaliz/offload_handler.h"
#include "libnormaliz/automorph.h"

namespace libnormaliz {
using std::list;
using std::vector;
using std::map;
using std::pair;
using boost::dynamic_bitset;

template<typename Integer> class Cone;
template<typename Integer> class SimplexEvaluator;
template<typename Integer> class CandidateList;
template<typename Integer> class Candidate;
template<typename Integer> class Simplex;
template<typename Integer> class Collector;
template<typename Integer> class Cone_Dual_Mode;
template<typename Integer> class FACETDATA;

template<typename Integer>
class Full_Cone {

    friend class Cone<Integer>;
    friend class SimplexEvaluator<Integer>;
    friend class CandidateList<Integer>;
    friend class Candidate<Integer>;
    friend class Collector<Integer>;
    
public:
    
    int omp_start_level; // records the omp_get_level() when the computation is started
                         // recorded at the start of the top cone constructor and the compute functions
                         // compute and dualize_cone
    size_t dim;
    size_t level0_dim; // dim of cone in level 0 of the inhomogeneous case
    size_t module_rank;  // rank of solution module over level 0 monoid in the inhomogeneous case
    size_t nr_gen;
    // size_t hyp_size; // not used at present
    Integer index; // index of full lattice over lattice of generators
    
    bool verbose;
    
    bool keep_convex_hull_data;
    
    bool pointed;
    bool is_simplicial;
    bool deg1_generated_computed;
    bool deg1_generated;
    bool deg1_extreme_rays;
    bool deg1_triangulation;
    bool deg1_hilbert_basis;
    bool inhomogeneous;
    
    // control of what to compute (set from outside)
    bool explicit_full_triang; // indicates whether full triangulation is asked for without default mode
    bool explicit_h_vector; // to distinguish it from being set via default mode
    bool do_determinants;
    bool do_multiplicity;
    bool do_integrally_closed;
    bool do_Hilbert_basis;
    bool do_deg1_elements;
    bool do_h_vector;
    bool keep_triangulation;
    bool do_Stanley_dec;
    bool do_default_mode;
    bool do_class_group;
    bool do_module_gens_intcl;
    bool do_module_rank;
    bool do_cone_dec;

    bool exploit_automs_mult;
    bool exploit_automs_vectors;
    bool do_automorphisms;
    
    bool do_hsop;
    bool do_extreme_rays;
    bool do_pointed;
    bool do_triangulation_size;
    
    // algorithmic variants
    bool do_approximation;
    bool do_bottom_dec;
    bool suppress_bottom_dec;
    bool keep_order;

    
    bool hilbert_basis_rec_cone_known;

    // control of triangulation and evaluation
    bool do_triangulation;
    bool do_partial_triangulation;
    bool do_only_multiplicity;
    bool stop_after_cone_dec;
    bool do_evaluation;
    bool triangulation_is_nested;
    bool triangulation_is_partial;

    // type of definition of automorphism group
    AutomParam::Quality quality_of_automorphisms;

    // internal helper control variables
    bool use_existing_facets;  // in order to avoid duplicate computation of already computed facets
    bool do_subdivision_points;
    bool do_excluded_faces;
    bool no_descent_to_facets; // primal algorithm must be applied to God_Father
    bool do_only_supp_hyps_and_aux;
    bool do_all_hyperplanes;  // controls whether all support hyperplanes must be computed
    bool use_bottom_points;
    ConeProperties is_Computed;    
    bool has_generator_with_common_divisor;
    
    long autom_codim_vectors; // bound for the descent to faces in algorithms using automorphisms
    long autom_codim_mult; // bound ditto for multiplicity
    Integer HB_bound; // only degree bound used in connection with automorphisms
                      // to discard vectors quickly 
                      
    bool time_measured;
    bool don_t_add_hyperplanes; // blocks the addition of new hyperplanes during time measurement
    bool take_time_of_large_pyr; // if true, the time of large pyrs is measured
    vector<clock_t> time_of_large_pyr;
    vector<clock_t> time_of_small_pyr;
    vector<size_t> nr_pyrs_timed;

    // data of the cone (input or output)
    vector<Integer> Truncation;  //used in the inhomogeneous case to suppress vectors of level > 1
    vector<Integer> Norm;  // is Truncation or Grading, used to "simplify" renf_elem_vectors
    Integer TruncLevel; // used for approximation of simplicial cones
    vector<Integer> Grading;
    vector<Integer> Sorting;
    mpq_class multiplicity;
#ifdef ENFNORMALIZ
    renf_elem_class renf_multiplicity;
#endif
    Matrix<Integer> Generators;
    set<vector<Integer> > Generator_Set; //the generators as a set (if needed)
    Matrix<nmz_float> Generators_float; // floatung point approximations to the generators
    vector<key_t> PermGens;  // stores the permutation of the generators created by sorting
    vector<bool> Extreme_Rays_Ind;
    Matrix<Integer> Support_Hyperplanes;
    Matrix<Integer> HilbertBasisRecCone;
    Matrix<Integer> Subcone_Support_Hyperplanes; // used if *this computes elements in a subcone, for example in approximation
    Matrix<Integer> Subcone_Equations;
    vector<Integer> Subcone_Grading;
    size_t nrSupport_Hyperplanes;
    list<vector<Integer> > Hilbert_Basis;
    vector<Integer> Witness;    // for not integrally closed
    Matrix<Integer> Basis_Max_Subspace; // a basis of the maximal linear subspace of the cone --- only used in connection with dual mode
    list<vector<Integer> > ModuleGeneratorsOverOriginalMonoid;
    CandidateList<Integer> OldCandidates,NewCandidates,HBRC,ModuleGensDepot;   // for the Hilbert basis
    // HBRC is for the Hilbert basis of the recession cone if provided, ModuleGensDepot for the collected module 
    // generators in this case
    size_t CandidatesSize;
    list<vector<Integer> > Deg1_Elements;
    HilbertSeries Hilbert_Series;
    vector<Integer> gen_degrees;  // will contain the degrees of the generators
    vector<long> gen_degrees_long;  // will contain the degrees of the generators as long (for h-vector)
    Integer shift; // needed in the inhomogeneous case to make degrees positive
    vector<Integer> gen_levels;  // will contain the levels of the generators (in the inhomogeneous case)
    size_t TriangulationBufferSize;          // number of elements in Triangulation, for efficiency
    list< SHORTSIMPLEX<Integer> > Triangulation;       // triangulation of cone
    list< SHORTSIMPLEX<Integer> > TriangulationBuffer; // simplices to evaluate
    list< SimplexEvaluator<Integer> > LargeSimplices; // Simplices for internal parallelization
    Integer detSum;                  // sum of the determinants of the simplices
    list< STANLEYDATA_int> StanleyDec; // Stanley decomposition
    vector<Integer> ClassGroup;  // the class group as a vector: ClassGroup[0]=its rank, then the orders of the finite cyclic summands
    
    Matrix<Integer> ProjToLevel0Quot;  // projection matrix onto quotient modulo level 0 sublattice    

    // ************************** Data for convex hull computations ****************************
    vector<size_t> HypCounter; // counters used to give unique number to hyperplane
                               // must be defined thread wise to avoid critical
                               
    vector<bool> in_triang;  // intriang[i]==true means that Generators[i] has been actively inserted
    vector<key_t> GensInCone;    // lists the generators completely built in
    size_t nrGensInCone;    // their number
    
    vector<size_t> Comparisons; // at index i we note the total number of comparisons 
                               // of positive and negative hyperplanes needed for the first i generators
    size_t nrTotalComparisons; // counts the comparisons in the current computation
        
    /* struct FACETDATA<Integer>{
        vector<Integer> Hyp;               // linear form of the hyperplane
        boost::dynamic_bitset<> GenInHyp;  // incidence hyperplane/generators
        Integer ValNewGen;                 // value of linear form on the generator to be added
        size_t BornAt;                      // number of generator (in order of insertion) at which this hyperplane was added,, counting from 0
        size_t Ident;                      // unique number identifying the hyperplane (derived from HypCounter)
        size_t Mother;                     // Ident of positive mother if known, 0 if unknown
        bool is_positive_on_all_original_gens;
        bool is_negative_on_some_original_gen;
        bool simplicial;                   // indicates whether facet is simplicial
    };*/

    list<FACETDATA<Integer>> Facets;  // contains the data for Fourier-Motzkin and extension of triangulation
    size_t old_nr_supp_hyps; // must be remembered since Facets gets extended before the current generators is finished
    
    // ******************************************************************************************
    
    // Pointer to the cone by which the Full_Cone has been constructed (if any)
    // Cone<Integer>* Creator;
    Matrix<Integer> Embedding; // temporary solution

    // the absolute top cone in recursive algorithms where faces are evalutated themselves
    Full_Cone<Integer>* God_Father;
    
    // data relating a pyramid to its ancestores
    Full_Cone<Integer>* Top_Cone; // reference to cone on top level relative to pyramid formation

    vector<key_t> Top_Key;        // indices of generators w.r.t Top_Cone
    Full_Cone<Integer>* Mother;   // reference to the mother of the pyramid
    vector<key_t> Mother_Key;     // indices of generators w.r.t Mother
    size_t apex; // indicates which generator of mother cone is apex of pyramid
    int pyr_level;  // -1 for top cone, increased by 1 for each level of pyramids

    int descent_level; // measures the decent in recursive algorithms that exploit compute_automorphisms
		       // 0 for God_father, increases by 1 with each passge to a facet
    
    Isomorphism_Classes<Integer> FaceClasses;
    
    vector<bool> IsLarge; // additional information whether pyramid is large 


    // control of pyramids, recusrion and parallelization
    bool is_pyramid; // false for top cone
    long last_to_be_inserted; // good to know in case of do_all_hyperplanes==false
    bool recursion_allowed;  // to allow or block recursive formation of pytamids
    bool multithreaded_pyramid; // indicates that this cone is computed in parallel threads
    bool tri_recursion; // true if we have gone to pyramids because of triangulation
   
    // storage for subpyramids
    size_t store_level; // the level on which daughters will be stored  
    deque< list<vector<key_t> > > Pyramids;  //storage for pyramids
    deque<size_t> nrPyramids; // number of pyramids on the various levels
    deque<bool> Pyramids_scrambled; // only used for mic

    // data that can be used to go out of build_cone and return later (not done at present)
    // but also useful at other places
    // long nextGen; // the next generator to be processed
    long lastGen; // the last generator processed
    
    // Helpers for triangulation and Fourier-Motzkin
    vector<typename list < SHORTSIMPLEX<Integer> >::iterator> TriSectionFirst;   // first simplex with lead vertex i
    vector<typename list < SHORTSIMPLEX<Integer> >::iterator> TriSectionLast;     // last simplex with lead vertex i
    list<FACETDATA<Integer>> LargeRecPyrs; // storage for large recusive pyramids given by basis of pyramid in mother cone
    
    list< SHORTSIMPLEX<Integer> > FreeSimpl;           // list of short simplices already evaluated, kept for recycling
    vector<list< SHORTSIMPLEX<Integer> > > FS;         // the same per thread
    vector< Matrix<Integer> > RankTest;
    vector< Matrix<nmz_float> > RankTest_float;// helper matrices for rank test
    
    // helpers for evaluation
    vector< SimplexEvaluator<Integer> > SimplexEval; // one per thread
    vector< Collector<Integer> > Results; // one per thread
    vector<Integer> Order_Vector;  // vector for the disjoint decomposition of the cone
#ifdef NMZ_MIC_OFFLOAD
    MicOffloader<long long> mic_offloader;
#endif
void try_offload_loc(long place,size_t max_level);

    template<typename IntegerCone>
    void restore_previous_vcomputation(CONVEXHULLDATA<IntegerCone>& ConvHullData);


    // defining semiopen cones
    Matrix<Integer> ExcludedFaces;
    map<boost::dynamic_bitset<>, long> InExCollect;

    // statistics
    size_t totalNrSimplices;   // total number of simplices evaluated
    size_t nrSimplicialPyr;
    size_t totalNrPyr;
    
    size_t start_from;
    
    size_t AdjustedReductionBound;
    
    bool is_approximation;
    
    AutomorphismGroup<Integer> Automs;

    bool is_global_approximation; // true if approximation is defined in Cone

    vector<vector<key_t> > approx_points_keys;
    Matrix<Integer> OriginalGenerators;

    Integer VolumeBound; //used to stop computation of approximation if simplex of this has larger volume
    
    long renf_degree;

/* ---------------------------------------------------------------------------
 *              Private routines, used in the public routines
 * ---------------------------------------------------------------------------
 */
    void number_hyperplane(FACETDATA<Integer>& hyp, const size_t born_at, const size_t mother);
    bool is_hyperplane_included(FACETDATA<Integer>& hyp);
    vector<Integer> FM_comb(const vector<Integer>& Pos, const Integer& PosVal, 
                    const vector<Integer>& Neg, const Integer& NegVal,bool extract_gcd=true);
    void add_hyperplane(const size_t& new_generator, const FACETDATA<Integer>& positive,const FACETDATA<Integer>& negative,
                     list<FACETDATA<Integer>>& NewHyps, bool known_to_be_simplicial);
    void extend_triangulation(const size_t& new_generator);
    void find_new_facets(const size_t& new_generator);
    void process_pyramids(const size_t new_generator,const bool recursive);
    void process_pyramid(const vector<key_t>& Pyramid_key, 
                      const size_t new_generator, const size_t store_level, Integer height, const bool recursive,
                      typename list< FACETDATA<Integer>>::iterator hyp, size_t start_level);
    void select_supphyps_from(const list<FACETDATA<Integer>>& NewFacets, const size_t new_generator, 
                      const vector<key_t>& Pyramid_key, const vector<bool>& Pyr_in_triang);
    bool check_pyr_buffer(const size_t level);
    void evaluate_stored_pyramids(const size_t level);
    void match_neg_hyp_with_pos_hyps(const FACETDATA<Integer>& hyp, size_t new_generator,list<FACETDATA<Integer>*>& PosHyps, 
                                     boost::dynamic_bitset<>& Zero_P, vector<list<boost::dynamic_bitset<> > >& Facets_0_1);
    void collect_pos_supphyps(list<FACETDATA<Integer>*>& PosHyps, boost::dynamic_bitset<>& Zero_P, size_t& nr_pos);
    void evaluate_rec_pyramids(const size_t level);
    void evaluate_large_rec_pyramids(size_t new_generator);

    void find_and_evaluate_start_simplex();
    // Simplex<Integer> find_start_simplex() const;
    vector<key_t>  find_start_simplex() const;
    void store_key(const vector<key_t>&, const Integer& height, const Integer& mother_vol,
                                  list< SHORTSIMPLEX<Integer> >& Triangulation);
    void find_bottom_facets();                                  
    vector<list<vector<Integer> > > latt_approx(); // makes a cone over a lattice polytope approximating "this"
    void convert_polyhedron_to_polytope();
    void compute_elements_via_approx(list<vector<Integer> >& elements_from_approx); // uses the approximation
    void compute_deg1_elements_via_approx_global(); // deg 1 elements from the approximation
    void compute_deg1_elements_via_projection_simplicial(const vector<key_t>& key); // for a simplicial subcone by projecion
    void compute_sub_div_elements(const Matrix<Integer>& gens,list<vector<Integer> >& sub_div_elements,
    bool best_point=false); //computes subdividing elements via approximation
    void select_deg1_elements(const Full_Cone& C);
//    void select_Hilbert_Basis(const Full_Cone& C); //experimental, unused
    
    void build_top_cone(); 
    void build_cone();
    void get_supphyps_from_copy(bool from_scratch, bool with_extreme_rays=false);   // if evealuation starts before support hyperplanes are fully computed
    void update_reducers(bool forced=false);   // update list of reducers after evaluation of simplices
    

    bool is_reducible(list<vector<Integer> *> & Irred, const vector<Integer> & new_element);
    void global_reduction();

    vector<Integer> compute_degree_function() const;
    
    Matrix<Integer> select_matrix_from_list(const list<vector<Integer> >& S,vector<size_t>& selection);

    bool contains(const vector<Integer>& v);
    bool subcone_contains(const vector<Integer>& v);
    bool contains(const Full_Cone& C);
    void extreme_rays_and_deg1_check();
    void find_grading();
    void find_grading_inhom();
    void check_given_grading();
    void disable_grading_dep_comp();
    void set_degrees();
    void set_levels(); // for truncation in the inhomogeneous case
    void find_module_rank(); // finds the module rank in the inhom case
    void find_module_rank_from_HB();
    void find_module_rank_from_proj();  // used if Hilbert basis is not computed
    void find_level0_dim(); // ditto for the level 0 dimension
    void find_level0_dim_from_HB(); // from the Hilbert basis (after dual mode)
    void sort_gens_by_degree(bool triangulate);
    // void compute_support_hyperplanes(bool do_extreme_rays=false);
    bool check_evaluation_buffer();
    bool check_evaluation_buffer_size();
    void prepare_old_candidates_and_support_hyperplanes();
    void evaluate_triangulation();
    void evaluate_large_simplices();
    void evaluate_large_simplex(size_t j, size_t lss);
    void transfer_triangulation_to_top();
    void primal_algorithm();
    void primal_algorithm_initialize();
    void primal_algorithm_finalize();
    void primal_algorithm_set_computed();
    void finish_Hilbert_series();
    void make_module_gens();
    void make_module_gens_and_extract_HB();
    void remove_duplicate_ori_gens_from_HB();
    void compute_class_group();
    void compose_perm_gens(const vector<key_t>& perm);
    void check_grading_after_dual_mode();

    void minimize_support_hyperplanes();   
    void compute_extreme_rays(bool use_facets=false);
    void compute_extreme_rays_compare(bool use_facets);
    void compute_extreme_rays_rank(bool use_facets);
    void select_deg1_elements();

    void check_pointed();
    void deg1_check();
    void check_deg1_extreme_rays();
    void check_deg1_hilbert_basis();

    void compute_multiplicity();
    
    void minimize_excluded_faces();
    void prepare_inclusion_exclusion();

    void set_implications();
    void set_primal_algorithm_control_variables();
    void reset_tasks();
    void deactivate_completed_tasks();
    void addMult(Integer& volume, const vector<key_t>& key, const int& tn); // multiplicity sum over thread tn
    
    void check_simpliciality_hyperplane(const FACETDATA<Integer>& hyp) const;
    void check_facet(const FACETDATA<Integer>& Fac, const size_t& new_generator) const; // debugging routine
    void set_simplicial(FACETDATA<Integer>& hyp);    

    void compute_hsop();
    void heights(list<vector<key_t> >& facet_keys,list<pair<boost::dynamic_bitset<>,size_t> > faces, size_t index,vector<size_t>& ideal_heights, size_t max_dim);
    
    void start_message();
    void end_message();
    
    void set_zero_cone();
    
    void compute_automorphisms(size_t nr_special_gens=0);
    void compute_by_automorphisms();
    mpq_class facet_multiplicity(const vector<key_t>& facet_key);
    void compute_multiplicity_via_automs();
    vector<vector<key_t> > get_facet_keys_for_orbits(const vector<Integer>& fixed_point,bool with_orbit_sizes);
    vector<Integer> get_fixed_point(size_t nr_cone_points);
    void compute_HB_via_automs();
    vector<Integer> replace_fixed_point_by_generator(const vector<Integer>& fixed_point,const key_t facet_nr,
                            const vector<Integer>& help_grading);
    void compute_Deg1_via_automs();
    void get_cone_over_facet_vectors(const vector<Integer>& fixed_point, const vector<key_t>& facet_key, const key_t facet_nr,
                                     list<vector<Integer> >& facet_vectors);
    Matrix<Integer>  push_supphyps_to_cone_over_facet(const vector<Integer>& fixed_point, const key_t facet_nr);
    void import_HB_from(const IsoType<Integer>& copy);
    bool check_extension_to_god_father();
    void compute_multiplicity_via_recession_cone();
    void copy_autom_params(const Full_Cone<Integer>& C);
    
    void recursive_revlex_triangulation(vector<key_t> simplex_so_far,const vector<key_t>& gens_in_face, 
                                        const vector<typename list<FACETDATA<Integer>>::const_iterator>& mother_facets,size_t dim );
    void make_facets();
    void revlex_triangulation();

    double rank_time();
    double cmp_time();
    double ticks_comp_per_supphyp;
    double ticks_rank_per_row;
    double ticks_per_cand;
    double ticks_quot;
    
    void small_vs_large(const size_t new_generator); // compares computation times of small vs. large pyramids


#ifdef NMZ_MIC_OFFLOAD
    void try_offload(size_t max_level);
#else
    void try_offload(size_t max_level) {};
#endif


/*---------------------------------------------------------------------------
 *                      Constructors
 *---------------------------------------------------------------------------
 */
    Full_Cone(); // default constructor
    Full_Cone(const Matrix<Integer>& M, bool do_make_prime=true);            //main constructor
    Full_Cone(Cone_Dual_Mode<Integer> &C);            // removes data from the argument!
    Full_Cone(Full_Cone<Integer>& C, const vector<key_t>& Key); // for pyramids

/*---------------------------------------------------------------------------
 *                      Data access
 *---------------------------------------------------------------------------
 */
    void print() const;             //to be modified, just for tests
    size_t getDimension() const;       
    size_t getNrGenerators() const;    
    bool isPointed() const;
    bool isDeg1ExtremeRays() const;
    bool isDeg1HilbertBasis() const;
    vector<Integer> getGrading() const; 
    mpq_class getMultiplicity() const;
    Integer getShift()const;
    size_t getModuleRank()const;
    const Matrix<Integer>& getGenerators() const;
    vector<bool> getExtremeRays() const;
    size_t getNrExtremeRays() const;
    Matrix<Integer> getSupportHyperplanes() const;
    Matrix<Integer> getHilbertBasis() const;
    Matrix<Integer> getModuleGeneratorsOverOriginalMonoid()const;
    Matrix<Integer> getDeg1Elements() const;
    vector<Integer> getHVector() const;
    Matrix<Integer> getExcludedFaces()const;
    
    bool isComputed(ConeProperty::Enum prop) const; 


/*---------------------------------------------------------------------------
 *              Computation Methods
 *---------------------------------------------------------------------------
 */
    void dualize_cone(bool print_message=true);
    void support_hyperplanes();

    void compute();

    /* adds generators, they have to lie inside the existing cone */
    void add_generators(const Matrix<Integer>& new_points);

    void dual_mode();

    void error_msg(string s) const;
};
//class end *****************************************************************


template<typename Integer>
template<typename IntegerCone>
void Full_Cone<Integer>::restore_previous_vcomputation(CONVEXHULLDATA<IntegerCone>& ConvHullData){
    
    /* ConvHullData.Generators.pretty_print(cout);
    cout << "===============" << endl;
    Generators.pretty_print(cout);
    cout << "===============" << endl;*/

    swap(ConvHullData.HypCounter,HypCounter);
    start_from=ConvHullData.Generators.nr_of_rows();
    /* for(size_t i=0;i<start_from;++i)
        in_triang[i]=ConvHullData.in_triang[i];*/
    swap(ConvHullData.in_triang,in_triang);
    in_triang.resize(nr_gen);
    swap(ConvHullData.GensInCone,GensInCone);
    nrGensInCone=ConvHullData.nrGensInCone;
    swap(ConvHullData.Comparisons,Comparisons);
    nrTotalComparisons=ConvHullData.nrTotalComparisons;
    old_nr_supp_hyps=ConvHullData.old_nr_supp_hyps;
    
    for(auto Fac=ConvHullData.Facets.begin();Fac!=ConvHullData.Facets.end();++Fac){
        FACETDATA<Integer> Ret;
        if(ConvHullData.is_primal)
            ConvHullData.SLR.convert_to_sublattice_dual(Ret.Hyp,Fac->Hyp);
        else
            ConvHullData.SLR.convert_to_sublattice(Ret.Hyp,Fac->Hyp);
        swap(Ret.GenInHyp,Fac->GenInHyp);
        Ret.GenInHyp.resize(nr_gen);
        // convert(Ret.ValNewGen,Fac->ValNewGen);
        Ret.BornAt=Fac->BornAt;
        Ret.Ident=Fac->Ident;
        Ret.Mother=Fac->Mother;
        Ret.is_positive_on_all_original_gens=Fac->is_positive_on_all_original_gens;
        Ret.is_negative_on_some_original_gen=Fac->is_negative_on_some_original_gen;
        Ret.simplicial=Fac->simplicial;
        
        Facets.push_back(Ret);        
    }
    
    for(size_t i=0;i<ConvHullData.Generators.nr_of_rows();++i){
        if(ConvHullData.is_primal)
            ConvHullData.SLR.convert_to_sublattice(Generators[i], ConvHullData.Generators[i]);
        else
            ConvHullData.SLR.convert_to_sublattice_dual(Generators[i], ConvHullData.Generators[i]);
    }
    
    use_existing_facets=true;
}

//---------------------------------------------------------------------------

}

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

