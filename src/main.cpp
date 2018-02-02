#include "Application.h"

int main() {

	try {
		Application app;

		if (app.init() == false) return EXIT_FAILURE;

		app.run();
	} 
	catch (const rs::error& e) {
		fprintf(stderr, "RealSense: rs::error was thrown when calling %s(%s):\n", e.get_failed_function().c_str(), e.get_failed_args().c_str());
		fprintf(stderr, "\t%s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
//catch (std::exception& e) {
//	fprintf(stderr, "exception: %s\n", e.what());
//}