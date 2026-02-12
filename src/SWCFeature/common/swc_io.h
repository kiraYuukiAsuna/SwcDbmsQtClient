#ifndef SWC_IO_H
#define SWC_IO_H

#include "basic_c_fun/basic_surf_objs.h"

// Write a NeuronSWC list to a SWC/ESWC file.
// If fileSaveName ends with ".eswc", extended fields are written.
// fileOpenName is used to copy comment headers from the original file (optional).
bool export_list2file(QList<NeuronSWC>& lN, QString fileSaveName, QString fileOpenName = QString());

#endif // SWC_IO_H
