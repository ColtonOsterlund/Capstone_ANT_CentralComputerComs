#include "DestinationBox.h"

#include <iostream>

DestinationBox::DestinationBox(): package_type(PackageType::INVALID), num_packages_in_transit(0), num_packages_stored(0), id(-1) {}

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
    else if (package_type == package && (num_packages_in_transit + num_packages_stored < DESTINATION_BOX_SIZE)) {
        can_accept = true;
    }
    
    return can_accept;
}

bool DestinationBox::add_package(PackageType package)
{
    bool package_added = false;

    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
        return false;
    }

    if (package_type != PackageType::NONE && package != package_type) {
        std::cout << "Destination Box: Trying to add package of the wrong type" << std::endl;
    }
    else if (num_packages_in_transit + num_packages_stored >= DESTINATION_BOX_SIZE) {
        std::cout << "Destination Box: Trying to add package to a full box" << std::endl;
    }
    else {
        package_type = package;
        num_packages_in_transit++;
        package_added = true;
    }

    return package_added;
}

void DestinationBox::package_received()
{
    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
        return;
    }

    if (num_packages_in_transit == 0) {
        std::cout << "DestinationBox: Package was received but there are none in transit... Could be duplicate message" << std::endl;
    }
    else {
        num_packages_in_transit--;
        num_packages_stored++;
    }
}

bool DestinationBox::empty_box()
{
    if (package_type == PackageType::INVALID) {
        std::cout << "DestinationBox: Trying to perform operation on uninitialized box" << std::endl;
        return false;
    }

    if (num_packages_in_transit != 0) {
        std::cout << "Cannot clear box with contents in transit" << std::endl;
        return false;
    }

    package_type = PackageType::NONE;
    num_packages_in_transit = 0;
    num_packages_stored = 0;
    return true;
}

std::string DestinationBox::to_string()
{
    std::string s = "box_id: ";
    s.append(std::to_string(id));
    return s;
}
