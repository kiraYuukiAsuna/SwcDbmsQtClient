#include <QApplication>

#include "basic_c_fun/basic_surf_objs.h"
#include "swcutils.h"

int smain(int argc, char *argv[]) {
	QString inputPath =
		R"(C:\Users\KiraY\Desktop\A\18454_00106.swc_stamp_2024_09_30_09_55.ano.eswc)";
	QString outputPath =
		R"(C:\Users\KiraY\Desktop\A\18454_00106.swc_stamp_2024_09_30_09_55_unsorted.ano.eswc)";

	convertSWC2UnSorted(inputPath, outputPath);
	return 0;
}
