// Rcpp functions to optimize embedding from package umap

#include <Rcpp.h>
#include <math.h>
using namespace Rcpp;

// [[Rcpp::export]]
NumericVector clip4(NumericVector x, double inner, double outer) {
  int xlen = x.size();
  for (int i=0; i<xlen; i++) {
    x[i] *= inner;
    if (x[i]>4) {
      x[i] = 4;
    } else if (x[i]<-4) {
      x[i] = -4;
    }
    x[i] *= outer;
  }
  return x;
}

// [[Rcpp::export]]
NumericMatrix opt_epoch (NumericMatrix &embedding,
			      IntegerMatrix &pairs,
			      IntegerVector &adjust,
			      IntegerVector &nns,
			      NumericVector &abg,
			      double alpha) {

  // precompute some constants from the configuration
  double a = abg[0];
  double b = abg[1];
  double gamma = abg[2];
  double bm1 = b-1;
  double m2ab = -2*a*b;
  double p2gb = 2*gamma*b;
  int V = embedding.ncol();

  int numpairs = pairs.nrow();
  for (int i=0; i<numpairs; i++) {
    if (adjust[i]>0) {

      // identify current vertex to process
      int j = pairs(i, 0);
      NumericMatrix::Column current = embedding(_, j);

      // identify primary link
      int k = pairs(i, 1);
      NumericMatrix::Column other = embedding(_, k);

      // adjust primary link
      NumericVector codiff = current-other;
      double codist2 = sum(codiff*codiff);
      double gradcoeff = (m2ab*pow(codist2, bm1)) / (a*pow(codist2, b)+1);
      NumericVector gradd = clip4(codiff, gradcoeff, alpha);
      current = current + gradd;
 
      // adjust a set of other randomly selected links
      int nnsi = nns[i];
      NumericVector krandom = runif(nnsi, 0, V);
      for (int kindex = 0; kindex<nnsi; kindex++) {
        k = floor(krandom[kindex]);
    	NumericMatrix::Column other2 = embedding(_, k);
	    codiff = current-other2;
    	codist2 = sum(codiff*codiff);
	    gradcoeff = p2gb / ((0.001+codist2)*(a*pow(codist2, b)+1));
	    gradd = clip4(codiff, gradcoeff, alpha);
	    current = current + gradd;
      }
    }
  }
  return embedding;
}

