#include "DestinationBox.h"

#include <iostream>

DestinationBox::DestinationBox(): package_type(PackageType::INVALID), packages_in_transit(), packages_stored(), id(-1) {}

DestinationBox::DestinationBox(const DestinationBox& other) : id(other.id), package_type(other.package_type), packages_stored(other.packages_stored), packages_in_transit(other.packages_in_transit) {}

void DestinationBox::initialize_box(int box_id) {
    if (package_type == PackageType::INVALID) {
        package_type = PackageType::NONE;
        id = box_id;
    }
    else {
        std::cout << "DestinationBox: Trying to initialize a box more than once" << std::endl;
    }
}

bool DestinationBox::is_initialized() {
    return package_type != PackageType::INVALID;
}

bool DestinationBox::can_accept_package(PackageType package)
{
    bool can_accept = false;

    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
    }

    if (package_type == PackageType::NONE) {
        can_accept = true;
    }
    else if (package_type == package && (packages_in_transit.size() + packages_stored.size() < DESTINATION_BOX_SIZE)) {
        can_accept = true;
    }
    
    return can_accept;
}

bool DestinationBox::add_package(int package_id, PackageType package)
{
    bool package_added = false;

    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
        return false;
    }

    if (package_type != PackageType::NONE && package != package_type) {
        std::cout << "Destination Box: Trying to add package of the wrong type" << std::endl;
    }
    else if (packages_in_transit.size() + packages_stored.size() >= DESTINATION_BOX_SIZE) {
        std::cout << "Destination Box: Trying to add package to a full box" << std::endl;
    }
    else {
        if (packages_in_transit.find(package_id) != packages_in_transit.end()) {
            std::cout << "DestinationBox: Adding a duplicate package" << std::endl;
        }

        package_type = package;
        packages_in_transit.insert(package_id);
        package_added = true;
    }

    return package_added;
}

void DestinationBox::package_received(int package_id)
{
    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
        return;
    }

    if (packages_in_transit.size() == 0) {
        std::cout << "DestinationBox: Package was received but there are none in transit... Could be duplicate message" << std::endl;
    }
    else if (packages_in_transit.find(package_id) == packages_in_transit.end()) {
        std::cout << "DestinationBox: Package was received but it was not in transit for this box... Could be duplicate message" << std::endl;
    }
    else {
        packages_in_transit.erase(packages_in_transit.find(package_id));
        packages_stored.insert(package_id);
    }
}

void DestinationBox::clear_box(std::set<int>& removed_packages) {
    for (auto& package_id : removed_packages) {
        if (has_package_in_transit(package_id)) {
            packages_in_transit.erase(packages_in_transit.find(package_id));
        }
        else if (has_package_stored(package_id)) {
            packages_stored.erase(packages_stored.find(package_id));
        }
        else {
            std::cout << "DestinationBox: Package removed from box not found in transit or stored" << std::endl;
        }
    }
}


bool DestinationBox::empty_box()
{
    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
        return false;
    }

    package_type = PackageType::NONE;
    packages_in_transit.clear();
    packages_stored.clear();
    return true;
}

bool DestinationBox::has_package_stored(int package_id)
{
    if (packages_stored.find(package_id) != packages_stored.end()) {
        return true;
    }

    return false;
}

bool DestinationBox::has_package_in_transit(int package_id)
{
    if (packages_in_transit.find(package_id) != packages_in_transit.end()) {
        return true;
    }
    return false;
}

bool DestinationBox::remove_package(int package_id)
{
    if (has_package_stored(package_id)) {
        packages_stored.erase(packages_stored.find(package_id));
        return true;
    }
    return false;
}

std::string DestinationBox::to_string()
{
    std::string s = "box_id: ";
    s.append(std::to_string(id));
    s.append("Packages in transit: ");
    for (auto& pkg_id : packages_in_transit) {
        s.append(std::to_string(pkg_id));
        s.append(", ");
    }

    s.append("Packages stored: ");
    for (auto& pkg_id : packages_stored) {
        s.append(std::to_string(pkg_id));
        s.append(", ");
    }

    return s;
}
