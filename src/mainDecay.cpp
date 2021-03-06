
/** 
   Project:     SOFTSUSY 
   File:        main.cpp
   Author:      Ben Allanach 
   Manual:      B.C. Allanach, hep-ph/0104145, Comp. Phys. Comm. 143 (2002) 305 
                B.C. Allanach, M.A. Bernhardt, arXiv:0903.1805, Comp. Phys. 
		Commun. 181 (2010) 232-245
   Webpage:     http://allanach.home.cern.ch/allanach/softsusy.html
   Description: main calling program example: performs a scan of tan beta 
   (starting at CMSSM10.1.1) and prints out Higgs masses as a result in
   the format
       <tan beta>      <mh0>     <mA0>    <mH0>     <mH+/->
*/

#include "mainDecay.h"

using namespace softsusy;
/// switch on if you're trying to get it through PYTHIA; off otherwise
const bool PYTHIA = true;
const bool GMSB = false;

int main() {
  /// Sets up exception handling
  signal(SIGFPE, FPE_ExceptionHandler); 

  TOLERANCE = 1.0e-3;
  
  try {
    /// Sets format of output: 6 decimal places
    outputCharacteristics(6);

    void (*boundaryCondition)(MssmSoftsusy &, const DoubleVector &)=sugraBcs;
  /// Parameters used: CMSSM parameters
  double m12 = 500., a0 = 0., mGutGuess = 2.0e16, tanb = 10.0, m0 = 125.;
  int sgnMu = 1;      ///< sign of mu parameter 
  int numPoints = 100; ///< number of scan points

  QedQcd oneset;      ///< See "lowe.h" for default definitions parameters

  /// most important Standard Model inputs: you may change these and recompile
  double alphasMZ = 0.1187, mtop = 173.5, mbmb = 4.18;
  oneset.setAlpha(ALPHAS, alphasMZ);
  oneset.setPoleMt(mtop);
  oneset.setMbMb(mbmb);

  oneset.toMz();      ///< Runs SM fermion masses to MZ

  /// Print out the SM data being used, as well as quark mixing assumption and
  /// the numerical accuracy of the solution
  cout << "# Low energy data in SOFTSUSY: MIXING=" << MIXING << " TOLERANCE=" 
       << TOLERANCE << endl;

  int i; 

  /// Set limits of random scan
  int startn5 = 1, endn5 = 5;
  double startMmess = 1.0e5, endMmess = 1.0e6;
  double startx = 0.01, endx = 0.99;
  double startM0 = 1000., endM0 = 4000.;
  double startA0 = -4000., endA0 = 4000.;
  double startM12 = 1000., endM12 = 4000.;  
  double startTanb = 3.0, endTanb = 50.0;
  long idum = idummySave;

  char fname[80] = "tests";
  fstream fout(fname, ios::out);
  
  /// Cycle through different points in the scan
  for (i = 0; i<=numPoints; i++) {
    sgnMu = 1;
    
    tanb = (endTanb - startTanb) * ran1(idum) +
      startTanb; // set tan beta ready for the scan.
    double a0 = (endA0 - startA0) * ran1(idum) +
      startA0; // set tan beta ready for the scan.
    double m0 = (endM0 - startM0) * ran1(idum) +
      startM0; // set tan beta ready for the scan.
    double m12 = (endM12 - startM12) * ran1(idum) +
      startM12; // set tan beta ready for the scan.
    double mMess = (endMmess - startMmess) * ran1(idum) +
      startMmess; // set mMess ready for the scan
    int    n5 = (endn5 - startn5) * ran1(idum) +
      startn5; // set n5 ready for the scan
    double x = (endx - startx) * ran1(idum) + startx;
    double lambda = x * mMess;

    /// Preparation for calculation: set up object and input parameters
    MssmSoftsusy * r, m; 
    r = &m;
    
    DoubleVector pars(3); 
    pars(1) = m0; pars(2) = m12; pars(3) = a0;
    bool uni = true; // MGUT defined by g1(MGUT)=g2(MGUT)
    const char* modelIdent = "sugra"; 

    if (GMSB) {
      cout << "# Mmess=" << mMess << " n5=" << n5 << "lambda = " << lambda << "tanbeta=" << tanb << endl; 
      uni = false; 
      mGutGuess = mMess;
      boundaryCondition = &gmsbBcs;
      modelIdent = "gmsb";
      pars.setEnd(4);
      pars(1) = n5; pars(2) = mMess; pars(3) = lambda; pars(4) = 1.;
    } else
    cout << "# M0=" << m0 << " m12=" << m12 << " a0=" << a0 << " tanb="
	 << tanb << endl;
      
    threeBodyDecays = true;

    /// Calculate the spectrum
    r->lowOrg(boundaryCondition, mGutGuess, pars, sgnMu, tanb, oneset, uni);

    /// check the point in question is problem free: if so print the output
    if (!r->displayProblem().testSeriousProblem()) {
      NmssmSoftsusy a;
      double qMax = 0.; int num = 1;
      bool ewsbBCscale = false;

      if (PYTHIA) {
	r->lesHouchesAccordOutput(fout, modelIdent, pars, sgnMu, tanb, qMax,  
				  num, ewsbBCscale);
      	fout.precision(10);
      }
      else {
	r->lesHouchesAccordOutput(cout, modelIdent, pars, sgnMu, tanb, qMax,  
				  num, ewsbBCscale);      
	cout.precision(10);

     }
          
      if (PYTHIA) calculateDecays(fout, r, a, false);
      else calculateDecays(cout, r, a, false);      

      /// now, you've got to pass the output through PYTHIA and work out if it
      /// works alright
      if (PYTHIA) {
	char buff[500] = "cp tests ../pythia8186/examples/lesHouchesOutput; cd ../pythia8186/examples; rm pyOut; ./main24.exe > pyOut; cat pyOut >> ../../softsusy/pyOut; cd ../../softsusy";
      
	if (system(buff)) throw("Problem: error in PYTHIA // run\n");
      }
    }
    {
      /// print out what the problem(s) is(are)
      fout << "#" << r->displayProblem() << endl;
      fout.close();
    }
  }
  }
  catch(const string & a) { cout << a; return -1; }
  catch(const char * a) { cout << a; return -1; }
  catch(...) { cout << "Unknown type of exception caught.\n"; return -1; }

  return 0;
}
