/*!
  @author Shin'ichiro Nakaoka
*/

#include "../ProjectManager.h"
#include "../Item.h"
#include "PyItemList.h"
#include <cnoid/PyUtil>

using namespace std;
using namespace cnoid;
namespace py = pybind11;

namespace cnoid {

void exportPyProjectManager(py::module m)
{
    py::class_<ProjectManager>(m, "ProjectManager")
        .def_property_readonly_static(
            "instance", [](py::object){ return ProjectManager::instance(); }, py::return_value_policy::reference)
        .def("loadProject",
             &ProjectManager::loadProject, py::arg("filename"), py::arg("parentItem") = nullptr)
        .def("setCurrentProjectName", &ProjectManager::setCurrentProjectName)

        // deprecated
        .def_static("getInstance", &ProjectManager::instance, py::return_value_policy::reference)
        ;
}

}
