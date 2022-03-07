// Include MLPACK library
#include <mlpack/core.hpp>
// Pre-processing Data
#include <mlpack/core/data/split_data.hpp>
#include <mlpack/core/data/scaler_methods/mean_normalization.hpp>  
// Neural Network
#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/ann/ffn.hpp>
// Loss function
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
// Initialisation of the weights
#include <mlpack/methods/ann/init_rules/he_init.hpp>
// Activation function
#include <mlpack/methods/ann/layer/leaky_relu.hpp>
#include <mlpack/methods/ann/layer/linear.hpp>
// Optimisation function
#include <ensmallen.hpp>

// C++ References
#include <iostream>
#include <fstream>
#include <algorithm>


using namespace std;
using namespace mlpack;
using namespace mlpack::ann;
using namespace arma;
using namespace ens;


int main()
{
	// LOAD trained ML model
	FFN<MeanSquaredError<>, HeInitialization>  model;
	data::Load("trained_ML_model.xml", "model", model);


	// LOAD features of test building that needs to be predicted
	mat featuresTest; data::Load("test_inputBSD.txt", featuresTest);
	
	
	// SCALE features (same as features where model is trained on: "inputBSD.txt")
	mat features; data::Load("inputBSD.txt", features);
	mat scaledfeaturesTest;
	mlpack::data::MeanNormalization scaler;
	scaler.Fit(features);
	scaler.Transform(featuresTest, scaledfeaturesTest);
	scaledfeaturesTest.print();


	// PREDICT conformal model 
	mat prediction;
	model.Predict(scaledfeaturesTest, prediction);

	cout << fixed << setprecision(0);
	trans(prediction).raw_print(cout, "prediction Target: ");     cout << endl;


	// Write prediction to file 
	ofstream predictionf("predictionCF.txt", ios::trunc);
	for (int i = 0; i < prediction.size(); i++)
	{
		if (i < prediction.size() - 1) {
			predictionf << fixed << setprecision(0) << prediction[i] << endl;
		}
		else {
			predictionf << fixed << setprecision(0) << prediction[i];
		}
	}

	

	return 0;
}