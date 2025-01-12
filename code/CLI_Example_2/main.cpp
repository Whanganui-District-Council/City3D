//
// Created by Jin on 19/07/2022.
//
/*
Copyright (C) 2017  Liangliang Nan
https://3d.bk.tudelft.nl/liangliang/ - liangliang.nan@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "../model/point_set.h"
#include "../model/map.h"
#include "../model/map_io.h"
#include "../model/point_set_io.h"
#include "../method/method_global.h"
#include "../method/reconstruction.h"
#include "../basic/file_utils.h"


int main(int argc, char **argv) {
    ///ToDo: user may need to tune these parameters for their datasets
    Method::min_points = 40;
    Method::pixel_size = 0.15;

    const std::string directory = std::string(CITY3D_ROOT_DIR) + "/../data/building_instances";
    //get the file names of the input point clouds
    std::vector<std::string> all_file_names;
    FileUtils::get_files(directory, all_file_names, false);
    for (std::size_t i = 0; i < all_file_names.size(); ++i) {
        std::cout << "- processing " << i + 1 << "/" << all_file_names.size() << " file..." << std::endl;
        const std::string &file_name = all_file_names[i];

        if (file_name.find("ply") != std::string::npos) {

            // input point cloud file name
            const std::string input_cloud_file = file_name;
            std::cout << "\tread input point cloud from file: " << input_cloud_file << std::endl;

            // output mesh file name
            std::string result_file = file_name.substr(0, file_name.find(".ply")) + "_ReconstructedModel.obj";
            std::string footprint_file = file_name.substr(0, file_name.find(".ply")) + "_GeneratedFootprint.obj";

            // load input point cloud
            std::cout << "\tloading input point cloud data..." << std::endl;
            PointSet *pset = PointSetIO::read(input_cloud_file);
            if (!pset) {
                std::cerr << "\tfailed loading point cloud data from file: " << input_cloud_file << std::endl;
                return EXIT_FAILURE;
            }

            Reconstruction recon;

            // Step 1: generate footprint for the building
            Map *footprint = recon.generate_footprint(pset);
            MapIO::save(footprint_file, footprint);

            // Step 2: segmentation to obtain point clouds of individual buildings
            std::cout << "\tsegmenting individual buildings..." << std::endl;
            recon.segmentation(pset, footprint);

            // Step 3: extract planes from the point cloud of each building
            std::cout << "\textracting roof planes..." << std::endl;
            if (!recon.extract_roofs(pset, footprint)) {
                std::cerr << "\tno roofs could be extracted from the point cloud (" << input_cloud_file << ")"<< std::endl;
                continue;
            }

            // Step 4: reconstruct  the buildings one by one
            Map *result = new Map;
#ifdef HAS_GUROBI
            std::cout << "\treconstructing the buildings (using the Gurobi solver)..." << std::endl;
            bool status = recon.reconstruct(pset, footprint, result, LinearProgramSolver::GUROBI);
#else
            std::cout << "\treconstructing the buildings (using the SCIP solver)..." << std::endl;
            bool status = recon.reconstruct(pset, footprint, result, LinearProgramSolver::SCIP);
#endif

            if (status && result->size_of_facets() > 0) {
                if (MapIO::save(result_file, result))
                    std::cout << "\treconstruction result saved to file: " << result_file << std::endl;
                else
                    std::cerr << "\tfailed to save reconstruction result to file: " << result_file << std::endl;
            } else
                std::cerr << "\treconstruction failed. Input point cloud from file: " << input_cloud_file << std::endl;

            delete pset;
            delete result;
        }
    }

    return EXIT_SUCCESS;
}