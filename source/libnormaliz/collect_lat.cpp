/*
 * Normaliz
 * Copyright (C) 2007-2022  W. Bruns, B. Ichim, Ch. Soeger, U. v. d. Ohe
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * As an exception, when this program is distributed through (i) the App Store
 * by Apple Inc.; (ii) the Mac App Store by Apple Inc.; or (iii) Google Play
 * by Google Inc., then that store may impose any digital rights management,
 * device limits and/or redistribution restrictions that are required by its
 * terms of service.
 */

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "libnormaliz/collect_lat.h"
#include "libnormaliz/matrix.h"
#include "libnormaliz/input.h"

namespace libnormaliz {
using std::cout;
using std::endl;
using std::ifstream;

SplitData::SplitData(){
}

// c instructor with default values
SplitData::SplitData(const string& this_project){

    project = this_project;
    nr_split_patches = 1;
    split_patches.resize(nr_split_patches);
    split_patches[0] = 1;
    split_moduli.resize(nr_split_patches);
    split_moduli[0] = 1000;
    max_nr_splits_per_round = 1000;
    nr_splits_to_do = 1000;
    this_round = 0;
    this_refinement = 0;
}

void SplitData::read_data(const string& this_project){

    project = this_project;
    string name = project + ".split.data";

    ifstream split_control(name.c_str());
    if(!split_control.is_open())
        throw BadInputException(name + " does not exist");

    long nr_splitPatches_all_rounds = 1;
    split_control >> nr_split_patches;
    split_patches.resize(nr_split_patches);
    split_moduli.resize(nr_split_patches);
    this_split_residues.resize(nr_split_patches);
    for(long i = 0; i < nr_split_patches; ++i){
            split_control >> split_patches[i] >> split_moduli[i];
            nr_splitPatches_all_rounds *= split_moduli[i];
    }
    split_control >> max_nr_splits_per_round >> nr_splits_to_do;
    split_control >> this_round >> this_refinement;

    if(split_control.fail())
        throw BadInputException(name + " corrupt");

    if(this_refinement == 0){
        if( nr_splits_to_do != nr_splitPatches_all_rounds)
            throw BadInputException("Numbers of splits for all rounds does not fit");
    }
    else{
        refinement_residues.resize(nr_splits_to_do);
        for(size_t i= 0; i < nr_splits_to_do; ++i){ // skip entries in dist file
            refinement_residues[i].resize(nr_split_patches);
            for(size_t j = 0; j < nr_split_patches; ++j)
                split_control >> refinement_residues[i][j];
        }
        // Matrix<long>(refinement_residues).debug_print();
    }

    assert(split_control.good());

    split_control.close();
}

void SplitData::write_data(){

    string name = project + ".split.data";
    ofstream new_split_control(name.c_str());
    new_split_control << split_patches.size();
    for(size_t i = 0; i < split_patches.size(); ++i)
    new_split_control << " " << split_patches[i] << " " << split_moduli[i];
    new_split_control << endl;
    new_split_control << max_nr_splits_per_round << " " << nr_splits_to_do << endl;
    new_split_control << this_round << " " << this_refinement << endl;

    if(this_refinement == 0){
        new_split_control.close();
        return;
    }

    Matrix<long>(refinement_residues).pretty_print(new_split_control);
    new_split_control.close();
}

void SplitData::next_round(){

    SplitData def_data = *this;
    def_data.this_round++;
    def_data.write_data();
}

void SplitData::set_this_split(const long& given_split){
    if(given_split >= max_nr_splits_per_round)
        throw NoComputationException("Split index given by -X too large");
    this_split = given_split + this_round * max_nr_splits_per_round;
    split_res_rounds= this_split; // neede in cone for output
    if(this_split >= nr_splits_to_do)
        throw NoComputationException("Total split index too large");

    long res = this_split;
    if(this_refinement == 0){
        for(long i = 0; i < nr_split_patches; ++i){
            this_split_residues[i] = res % split_moduli[i];
            res /=  split_moduli[i];
        }
    }
    else
        this_split_residues = refinement_residues[this_split];
}

void next_round(const string& project) {

    SplitData our_split;
    our_split.read_data(project);
    // archive <project>.spli.data that was jiust read
    string command = "cp " + project + ".split.data " + project + "." + to_string(our_split.this_refinement) + "." + to_string(our_split.this_round) + ".split.data";
    int dummy = system(command.c_str());
    if(dummy > 0)
        throw NoComputationException("Problem in archiving <project.split.data");
    // now mwrite updated split data
    our_split.this_round++;
    our_split.write_data();
}

void collect_lat(const string& project) {

    string name;

    SplitData our_split;
    our_split.read_data(project);
    // archive <project>.spli.data that was jiust read
    string command = "cp " + project + ".split.data " + project + "." + to_string(our_split.this_refinement) + "." + to_string(our_split.this_round) + ".split.data";
    int dummy = system(command.c_str());
    if(dummy > 0)
        throw NoComputationException("Problem in archiving <project.split.data");

    if(verbose)
        verboseOutput() << "Collecting lattice points from " << our_split.nr_splits_to_do << " lat files" << endl;

    Matrix<long long> TotalLat;
    bool first = true;

    // First we must vread what has been compouted in previous refinements
    name = project + ".total.lat";
    ifstream lat_in(name.c_str());
    if(lat_in.is_open()){
        TotalLat = readMatrix<long long>(name);
        if(TotalLat.nr_of_rows() > 0){
                first = false;
        }
        lat_in.close();
    }

    vector<size_t> NotDone;


    // Now we read what has been compouted in the last refinement
    // and register the parts that have not been complete in NotDone
    for(size_t i = 0; i < our_split.nr_splits_to_do; ++i){
        name = project + "." + to_string(our_split.this_refinement) + "." +  to_string(i) + ".lat";
        if(verbose)
            verboseOutput()  << name << endl;

        const char* file_in = name.c_str();
        ifstream test_in;
        test_in.open(file_in, ifstream::in);
        if (!test_in.is_open()){
            if(verbose)
                verboseOutput() << name << "does not exist" << endl;
            NotDone.push_back(i);
            continue;
        }

        Matrix<long long> this_lat = readMatrix<long long>(name);
        if(this_lat.nr_of_rows() == 0)
            continue;
        if(first)
            TotalLat.resize(0, this_lat.nr_of_columns());
        first = false;
        TotalLat.append(this_lat);
    }

    name = global_project + ".total.lat";
    ofstream lat_out(name.c_str());
    TotalLat.print(lat_out);

    if(NotDone.size() == 0){
        if(verbose)
            verboseOutput() << "Computation complete" << endl;
        return;
    }

    if(verbose)
        verboseOutput() << "Computation NOT complete" << endl;
    SplitData new_split_data = our_split;
    size_t nr_sub_splits = 2;
    if(our_split.max_nr_splits_per_round/NotDone.size() > 2)
        nr_sub_splits = our_split.max_nr_splits_per_round/NotDone.size();
    new_split_data.nr_splits_to_do = NotDone.size() * nr_sub_splits;
    new_split_data.split_patches.push_back(our_split.split_patches.back() + 1);
    new_split_data.nr_split_patches++;
    new_split_data.split_moduli.push_back(nr_sub_splits);
    new_split_data.this_refinement++;
    new_split_data.this_round = 0;

    if(verbose)
        verboseOutput() << "Scheduling refinement" << endl;
    if(verbose)
        verboseOutput() << nr_sub_splits << " subplits" << endl;

    vector<long> split_residues(our_split.nr_split_patches);
    new_split_data.refinement_residues.clear();
    for(auto& split_res:NotDone){
        if(our_split.this_refinement == 0){ // no previous refinement
            long res = split_res;
            for(long i = 0; i < our_split.nr_split_patches; ++i){
                split_residues[i] = res % our_split.split_moduli[i];
                res /=  our_split.split_moduli[i];
            }
        }
        else{
            split_residues = our_split.refinement_residues[split_res];
        }
        vector<long> extended_res = split_residues;
        extended_res.resize(extended_res.size() +1);
        for(long i = 0; i < nr_sub_splits; ++i){
            extended_res.back() = i;
            new_split_data.refinement_residues.push_back(extended_res);
        }
    }
    new_split_data.write_data();
}

}  // namespace libnormaliz
