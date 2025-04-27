#pragma once 
namespace sick {
	namespace easyranger {
		namespace calibration {
// WARNING
// Changing these settings could render previous versions of the calibration invalid.

// ====================================================================================
// Model settings
// ====================================================================================
// Select to have either 2 or 3 radial coefficients in the calibration model.
// Typically 2 should be enough, but for testing we have support for 3.
#define LENS_N_RADIAL 2

// This allows lens correction parameters to be at more manageable levels
// Eg. they more closely resemble a system where intrinsic parameters exist.
// Setting this to 1.0 give the effect of Coordinator calibration, Easy Ranger Single shot etc.
// Setting it to something like 1000-4000 gives lens correction coeffificents around +- 0.01
// instead of +-1e-10.
#define LENS_PRESCALER 1000.0 // Possibly we could make this depend on image size.

//#define LENS_PRESCALER 10000.0 // Possibly we could make this depend on image size.

// ====================================================================================
// Calibrator settings
// ====================================================================================

// Detection and model calculation
#define POLYNOMIAL_TYPE 0                   // 0 - Linear fit             1 - Use second degree refinement step.
#define LOSS_FUNCTION 0                     // 0 - Squared loss           1 - Huber loss
#define SHEIMPFLUG_MODEL 1                  // 0 - No Schemipflug model   1 - Centered               2 - Projective only

// Inverse calculation
#define INVERSE_METHOD 1                    // 1 - Best fit on lut area   2 - No padding,      3 - Matlab equivalent
#define INVERSE_SAMPLE_METHOD 1             // 1 - Model inverse (best)   2 - Forward splat
#define PERFORM_BORDER_POST_PROCESSING 0    // 0 - No border checking     1 - Project all lut points and verify they have values in the inverse.

// Polynomial compression
// SVD is slowest and most precise, QR without pivot is fastest, but gives the worst result.
//#define POLYGRID_DECOMPOSITION_TYPE 2 // 1 = SVD, 2 = QR Pivot, 3 = QR No pivot 

// Evaluate laser coverage either based on calibration points or segment existence.
// Using the calibration points should be more conservative than using the segments.
#define FOV_COVERAGE_TYPE 1           // 1 - Calibration points, 2 - Segments

		}
	}
}