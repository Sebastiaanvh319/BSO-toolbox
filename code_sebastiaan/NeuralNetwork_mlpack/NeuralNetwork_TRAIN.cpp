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

// Namespaces
using namespace std;
using namespace mlpack;
using namespace mlpack::ann;
using namespace arma;
using namespace ens;


int main()
{
	// LOAD DATA
	mat features;
	data::Load("inputBSD.txt", features, false, true);
	mat targets;
	data::Load("outputCF.txt", targets, false, true);

	// shuffle data
	mat data = move(join_cols(features, targets));
	mat shuffledData = shuffle(data,1);
	mat shuffledfeatures = shuffledData.rows(0,features.n_rows -1);
	mat shuffledtargets = shuffledData.rows(features.n_rows, features.n_rows + targets.n_rows -1);
	
	shuffledfeatures.brief_print();
	shuffledtargets.brief_print();

	// FEATURE SCALING
	mat shuffledscaledfeatures;
	mlpack::data::MeanNormalization scaler;
	scaler.Fit(shuffledfeatures);
	scaler.Transform(shuffledfeatures, shuffledscaledfeatures);
	shuffledscaledfeatures.brief_print();

	// SPLIT DATA
	// Training & Testing Data
	mat train_features, test_features, train_targets, test_targets;
	mlpack::data::Split(shuffledscaledfeatures, train_features, test_features, 0.2, false);			// 80% train, 20% test
	mlpack::data::Split(shuffledtargets , train_targets,  test_targets,  0.2, false);


	// BUILD ML MODEL
	FFN<MeanSquaredError<>, HeInitialization>  model;
	model.Add<Linear<> >(shuffledfeatures.n_rows, 180);					// Input layer --> Hidden layer
	model.Add<LeakyReLU<> >(0.0001);									// Activation function on hidden layer
	model.Add<Linear<> >(180, shuffledtargets.n_rows);					// Hidden layer --> Output layer
	model.Add<LeakyReLU<> >(0.0001);									// Activation function on output layer
	
	
	// TRAIN & TEST
	// Define optimisation function: Adam(Learning rate, Batch size)
	ens::Adam optimizer(0.01, 100);	
	// Mean Squared Error (MSE) is saved in text file
	ofstream MSE("MSE.txt", ios::trunc);
	// t: training iterations
	int t = 12;
	for (int i = 0; i < t; i++)
	{
		// Train
		model.Train(train_features, train_targets, optimizer);
		// Evaluate train & test
		cout << i+1 << ") Evaluate Train: " << model.Evaluate(train_features, train_targets) << endl;
		MSE << model.Evaluate(train_features, train_targets) << "," << model.Evaluate(test_features, test_targets) << endl;
		cout << i+1 << ") Evaluate Test : " << model.Evaluate(test_features, test_targets) << endl;
	}
	

	// SAVE model
	data::Save("trained_ML_model.xml", "model", model, false);
	


	return 0;
}