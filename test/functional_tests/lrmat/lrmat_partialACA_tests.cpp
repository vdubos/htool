#include <iostream>
#include <complex>
#include <vector>

#include <htool/cluster.hpp>
#include <htool/lrmat.hpp>
#include <htool/partialACA.hpp>


using namespace std;
using namespace htool;


class MyMatrix: public IMatrix<double>{
	const vector<R3>& p1;
	const vector<R3>& p2;

public:
	MyMatrix(const vector<R3>& p10,const vector<R3>& p20 ):IMatrix<double>(p10.size(),p20.size()),p1(p10),p2(p20) {}
	 double get_coef(const int& i, const int& j)const {return 1./(4*M_PI*norm2(p1[i]-p2[j]));}
	 std::vector<double> operator*(std::vector<double> a){
		std::vector<double> result(p1.size(),0);
		for (int i=0;i<p1.size();i++){
			for (int k=0;k<p2.size();k++){
				result[i]+=this->get_coef(i,k)*a[k];
			}
		}
		return result;
	 }
};


int main(){
	const int ndistance = 4;
	double distance[ndistance];
	distance[0] = 10; distance[1] = 20; distance[2] = 30; distance[3] = 40;
	SetNdofPerElt(1);
	bool test = 0;
	for(int idist=0; idist<ndistance; idist++)
	{
		cout << "Distance between the clusters: " << distance[idist] << endl;
		SetEpsilon(0.0001);
		srand (1);
		// we set a constant seed for rand because we want always the same result if we run the check many times
		// (two different initializations with the same seed will generate the same succession of results in the subsequent calls to rand)

		int nr = 500;
		int nc = 100;
		vector<int> Ir(nr); // row indices for the lrmatrix
		vector<int> Ic(nc); // column indices for the lrmatrix

		double z1 = 1;
		vector<R3>     p1(nr);
		vector<int>  tab1(nr);
		for(int j=0; j<nr; j++){
			Ir[j] = j;
			double rho = ((double) rand() / (double)(RAND_MAX)); // (double) otherwise integer division!
			double theta = ((double) rand() / (double)(RAND_MAX));
			p1[j][0] = sqrt(rho)*cos(2*M_PI*theta); p1[j][1] = sqrt(rho)*sin(2*M_PI*theta); p1[j][2] = z1;
			// sqrt(rho) otherwise the points would be concentrated in the center of the disk
			tab1[j]=j;
		}
		// p2: points in a unit disk of the plane z=z2
		double z2 = 1+distance[idist];
		vector<R3> p2(nc);
		vector<int> tab2(nc);
		for(int j=0; j<nc; j++){
            Ic[j] = j;
			double rho = ((double) rand() / (RAND_MAX)); // (double) otherwise integer division!
			double theta = ((double) rand() / (RAND_MAX));
			p2[j][0] = sqrt(rho)*cos(2*M_PI*theta); p2[j][1] = sqrt(rho)*sin(2*M_PI*theta); p2[j][2] = z2;
			tab2[j]=j;
		}

		Cluster t(p1,tab1); Cluster s(p2,tab2);
		MyMatrix A(p1,p2);

		// ACA with fixed rank
		int reqrank_max = 10;
		partialACA<double> A_partialACA_fixed(Ir,Ic,reqrank_max);
		A_partialACA_fixed.build(A,t,s);
		std::vector<double> partialACA_fixed_errors;
		for (int k = 0 ; k < A_partialACA_fixed.rank_of()+1 ; k++){
			partialACA_fixed_errors.push_back(Frobenius_absolute_error(A_partialACA_fixed,A,k));
		}

		cout << "Partial ACA with fixed rank" << endl;
		// Test rank
		cout << "rank : "<<A_partialACA_fixed.rank_of() << endl;
		test = test || !(A_partialACA_fixed.rank_of()==reqrank_max);

		// Test Frobenius errors
		test = test || !(partialACA_fixed_errors[partialACA_fixed_errors.size()-1]<1e-6);
		cout << "Errors with Frobenius norm : "<<partialACA_fixed_errors<<endl;

		// Test compression
		test = test || !(0.87<A_partialACA_fixed.compression() && A_partialACA_fixed.compression()<0.89);
		cout << "Compression rate : "<<A_partialACA_fixed.compression()<<endl;

		// Test mat vec prod
		std::vector<double> f(nc,1);
		double error=norm2(A*f-A_partialACA_fixed*f);
		test = test || !(error<1e-6);
		cout << "Errors on a mat vec prod : "<< error<<endl<<endl;



		// ACA automatic building
		partialACA<double> A_partialACA(Ir,Ic);
		A_partialACA.build(A,t,s);
		std::vector<double> partialACA_errors;
		for (int k = 0 ; k < A_partialACA.rank_of()+1 ; k++){
			partialACA_errors.push_back(Frobenius_absolute_error(A_partialACA,A,k));
		}

		cout << "Partial ACA" << endl;
		// Test Frobenius error
		test = test || !(partialACA_errors[A_partialACA.rank_of()]<GetEpsilon());
		cout << "Errors with Frobenius norm: "<<partialACA_errors<<endl;

		// Test compression rate
		test = test || !(0.93<A_partialACA.compression() && A_partialACA.compression()<0.96);
		cout << "Compression rate : "<<A_partialACA.compression()<<endl;

		// Test mat vec prod
		error = norm2(A*f-A_partialACA*f);
		test = test || !(error<GetEpsilon());
		cout << "Errors on a mat vec prod : "<< error<<endl<<endl<<endl;
	}

	return test;
}