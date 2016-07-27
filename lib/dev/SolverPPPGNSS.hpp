#pragma ident "$Id$"

/**
 * @file SolverPPPGNSS.hpp
 * Class to compute the PPP Solution.
 */

#ifndef GPSTK_SOLVERPPP_HPP
#define GPSTK_SOLVERPPP_HPP

//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//
//  Dagoberto Salazar - gAGE ( http://www.gage.es ). 2008, 2009, 2011
//
//============================================================================


#include <deque>
#include "CodeKalmanSolver.hpp"


namespace gpstk
{

      /** @addtogroup GPSsolutions */
      /// @ingroup math

      //@{

      /** This class computes the Precise Point Positioning (PPP) solution
       *  using a Kalman solver that combines ionosphere-free code and phase
       *  measurements.
       *
       * This class may be used either in a Vector- and Matrix-oriented way,
       * or with GNSS data structure objects from "DataStructures" class (much
       * more simple to use it this way).
       *
       * A typical way to use this class with GNSS data structures follows:
       *
       * @code
       *      // INITIALIZATION PART
       *
       *      // EBRE station nominal position
       *   Position nominalPos(4833520.192, 41537.1043, 4147461.560);
       *   RinexObsStream rin("ebre0300.02o");  // Data stream
       *
       *      // Load all the SP3 ephemerides files
       *   SP3EphemerisStore SP3EphList;
       *   SP3EphList.loadFile("igs11512.sp3");
       *   SP3EphList.loadFile("igs11513.sp3");
       *   SP3EphList.loadFile("igs11514.sp3");
       *
       *   NeillTropModel neillTM( nominalPos.getAltitude(),
       *                           nominalPos.getGeodeticLatitude(),
       *                           30 );
       *
       *      // Objects to compute the tropospheric data
       *   BasicModel basicM(nominalPos, SP3EphList);
       *   ComputeTropModel computeTropo(neillTM);
       *
       *      // More declarations here: ComputeMOPSWeights, SimpleFilter,
       *      // LICSDetector, MWCSDetector, SolidTides, OceanLoading, 
       *      // PoleTides, CorrectObservables, ComputeWindUp, ComputeLinear,
       *      // LinearCombinations, etc.
       *
       *      // Declare a SolverPPPGNSS object
       *   SolverPPPGNSS pppSolver;
       *
       *     // PROCESSING PART
       *
       *   gnssRinex gRin;
       *
       *   while(rin >> gRin)
       *   {
       *      try
       *      {
       *         gRin  >> basicM
       *               >> correctObs
       *               >> compWindup
       *               >> computeTropo
       *               >> linear1      // Compute combinations
       *               >> pcFilter
       *               >> markCSLI2
       *               >> markCSMW
       *               >> markArc
       *               >> linear2      // Compute prefit residuals
       *               >> phaseAlign
       *               >> pppSolver;
       *      }
       *      catch(...)
       *      {
       *         cerr << "Unknown exception at epoch: " << time << endl;
       *         continue;
       *      }
       *
       *         // Print results
       *      cout << time.DOYsecond()      << "  "; // Output field #1
       *      cout << pppSolver.solution[1] << "  "; // dx: Output field #2
       *      cout << pppSolver.solution[2] << "  "; // dy: Output field #3
       *      cout << pppSolver.solution[3] << "  "; // dz: Output field #4
       *      cout << pppSolver.solution[0] << "  "; // wetTropo: Out field #5
       *      cout << endl;
       *   }
       * @endcode
       *
       * The "SolverPPPGNSS" object will extract all the data it needs from the
       * GNSS data structure that is "gRin" and will try to solve the PPP
       * system of equations using a Kalman filter. It will also insert back
       * postfit residual data (both code and phase) into "gRin" if it
       * successfully solves the equation system.
       *
       * By default, it will build the geometry matrix from the values of
       * coefficients wetMap, dx, dy, dz and cdt, IN THAT ORDER. Please note
       * that the first field of the solution will be the estimation of the
       * zenital wet tropospheric component (or at least, the part that wasn't
       * modeled by the tropospheric model used).
       *
       * You may configure the solver to work with a NEU system in the class
       * constructor or using the "setNEU()" method.
       *
       * In any case, the "SolverPPPGNSS" object will also automatically add and
       * estimate the ionosphere-free phase ambiguities. The independent vector
       * is composed of the code and phase prefit residuals.
       *
       * This class expects some weights assigned to each satellite. That can
       * be achieved with objects from classes such as "ComputeIURAWeights",
       * "ComputeMOPSWeights", etc.
       *
       * If these weights are not assigned, then the "SolverPPPGNSS" object will
       * set a value of "1.0" to code measurements, and "weightFactor" to phase
       * measurements. The default value of "weightFactor" is "10000.0". This
       * implies that code sigma is 1 m, and phase sigma is 1 cm.
       *
       * By default, the stochastic models used for each type of variable are:
       *
       *    \li Coordinates are modeled as constants (StochasticModel).
       *    \li Zenital wet tropospheric component is modeled as a random walk
       *        (RandomWalkModel), with a qPrime value of 3e-8 m*m/s.
       *    \li Receiver clock is modeled as white noise (WhiteNoiseModel).
       *    \li Phase biases are modeled as white noise when cycle slips
       *        happen, and as constants between cycle slips
       *        (PhaseAmbiguityModel).
       *
       * You may change this assignment with methods "setCoordinatesModel()",
       * "setXCoordinatesModel()", "setYCoordinatesModel()",
       * "setZCoordinatesModel()", "setTroposphereModel()" and
       * "setReceiverClockModel()". However, you are not allowed to change the
       * phase biases stochastic model.
       *
       * For instance, in orden to use a 'full kinematic' mode we assign a white
       * noise model to all the coordinates:
       *
       * @code
       *      // Define a white noise model with 100 m of sigma
       *   WhiteNoiseModel wnM(100.0);
       *
       *      // Configure the solver to use this model for all coordinates
       *   pppSolver.setCoordinatesModel(&wnM);
       * @endcode
       *
       * Be aware, however, that you MUST NOT use this method to set a
       * state-aware stochastic model (like RandomWalkModel, for instance)
       * to ALL coordinates, because the results will certainly be erroneous.
       * Use this method ONLY with non-state-aware stochastic models like
       * 'StochasticModel' (constant coordinates) or 'WhiteNoiseModel'.
       *
       * In order to overcome the former limitation, this class provides methods
       * to set different, specific stochastic models for each coordinate, like:
       *
       * @code
       *      // Define a white noise model with 2 m of sigma for horizontal
       *      // coordinates (in this case, the solver is previously set to use
       *      // dLat, dLon and dH).
       *   WhiteNoiseModel wnHorizontalModel(2.0);
       *
       *      // Define a random walk model with 0.04 m*m/s of process spectral
       *      // density for vertical coordinates.
       *   RandomWalkModel rwVerticalModel(0.04);
       *
       *      // Configure the solver to use these models
       *   pppSolver.setXCoordinatesModel(&wnHorizontalModel);
       *   pppSolver.setYCoordinatesModel(&wnHorizontalModel);
       *   pppSolver.setZCoordinatesModel(&rwVerticalModel);
       * @endcode
       *
       *
       * \warning "SolverPPPGNSS" is based on a Kalman filter, and Kalman filters
       * are objets that store their internal state, so you MUST NOT use the
       * SAME object to process DIFFERENT data streams.
       *
       * @sa SolverBase.hpp, SolverLMS.hpp and CodeKalmanSolver.hpp for
       * base classes.
       *
       */
   class SolverPPPGNSS : public CodeKalmanSolver
   {
   public:

         /** Common constructor.
          *
          * @param useNEU   If true, will compute dLat, dLon, dH coordinates;
          *                 if false (the default), will compute dx, dy, dz.
          */
      SolverPPPGNSS( bool useNEU = false );
 
         /** Compute the PPP Solution of the given equations set.
          *
          * @param prefitResiduals   Vector of prefit residuals
          * @param designMatrix      Design matrix for the equation system
          * @param weightMatrix      Matrix of weights
          *
          * \warning A typical Kalman filter works with the measurements noise
          * covariance matrix, instead of the matrix of weights. Beware of this
          * detail, because this method uses the later.
          *
          * @return
          *  0 if OK
          *  -1 if problems arose
          */
      virtual int Compute( const Vector<double>& prefitResiduals,
                           const Matrix<double>& designMatrix,
                           const Matrix<double>& weightMatrix )
         throw(InvalidSolver);


         /** Compute the PPP Solution of the given equations set.
          *
          * @param prefitResiduals   Vector of prefit residuals
          * @param designMatrix      Design matrix for the equation system
          * @param weightVector      Vector of weights assigned to each
          *                          satellite.
          *
          * \warning A typical Kalman filter works with the measurements noise
          * covariance matrix, instead of the vector of weights. Beware of this
          * detail, because this method uses the later.
          *
          * @return
          *  0 if OK
          *  -1 if problems arose
          */
      virtual int Compute( const Vector<double>& prefitResiduals,
                           const Matrix<double>& designMatrix,
                           const Vector<double>& weightVector )
         throw(InvalidSolver);


         /** Returns a reference to a gnnsSatTypeValue object after
          *  solving the previously defined equation system.
          *
          * @param gData    Data object holding the data.
          */
      virtual gnssSatTypeValue& Process(gnssSatTypeValue& gData)
         throw(ProcessingException, SVNumException);


         /** Returns a reference to a gnnsRinex object after solving
          *  the previously defined equation system.
          *
          * @param gData    Data object holding the data.
          */
      virtual gnssRinex& Process(gnssRinex& gData)
         throw(ProcessingException, SVNumException);


         /** Resets the PPP internal Kalman filter.
          *
          * @param newState         System state vector
          * @param newErrorCov      Error covariance matrix
          *
          * \warning Take care of dimensions: In this case newState must be 6x1
          * and newErrorCov must be 6x6.
          *
          */
      virtual SolverPPPGNSS& Reset( const Vector<double>& newState,
                                    const Matrix<double>& newErrorCov )
      { kFilter.Reset( newState, newErrorCov ); return (*this); };


         /** Sets if a NEU system will be used.
          *
          * @param useNEU  Boolean value indicating if a NEU system will
          *                be used
          *
          */
      virtual SolverPPPGNSS& setNEU( bool useNEU );
        
	// Sets if use BeiDou, Galileo or Glonass
      virtual SolverPPPGNSS& setSatSystem( bool usingGPS,
                                           bool usingGLO,
                                           bool usingBDS,
                                           bool usingGAL );

         /** Get the weight factor multiplying the phase measurements sigmas.
          *  This factor is the code_sigma/phase_sigma ratio.
          */
      virtual double getWeightFactor(void) const
      { return std::sqrt(weightFactor); };


         /** Set the weight factor multiplying the phase measurement sigma
          *
          * @param factor      Factor multiplying the phase measurement sigma
          *
          * \warning This factor should be the code_sigma/phase_sigma ratio.
          * For instance, if we assign a code sigma of 1 m and a phase sigma
          * of 10 cm, the ratio is 100, and so should be "factor".
          */
      virtual SolverPPPGNSS& setWeightFactor(double factor)
      { weightFactor = (factor*factor); return (*this); };


         /// Get stochastic model pointer for dx (or dLat) coordinate
      StochasticModel* getXCoordinatesModel() const
      { return pCoordXStoModel; };


         /** Set coordinates stochastic model for dx (or dLat) coordinate
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    dx (or dLat) coordinate.
          */
      SolverPPPGNSS& setXCoordinatesModel(StochasticModel* pModel)
      { pCoordXStoModel = pModel; return (*this); };


         /// Get stochastic model pointer for dy (or dLon) coordinate
      StochasticModel* getYCoordinatesModel() const
      { return pCoordYStoModel; };


         /** Set coordinates stochastic model for dy (or dLon) coordinate
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    dy (or dLon) coordinate.
          */
      SolverPPPGNSS& setYCoordinatesModel(StochasticModel* pModel)
      { pCoordYStoModel = pModel; return (*this); };


         /// Get stochastic model pointer for dz (or dH) coordinate
      StochasticModel* getZCoordinatesModel() const
      { return pCoordZStoModel; };


         /** Set coordinates stochastic model for dz (or dH) coordinate
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    dz (or dH) coordinate.
          */
      SolverPPPGNSS& setZCoordinatesModel(StochasticModel* pModel)
      { pCoordZStoModel = pModel; return (*this); };


         /** Set a single coordinates stochastic model to ALL coordinates.
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    coordinates.
          *
          * @warning Do NOT use this method to set the SAME state-aware
          * stochastic model (like RandomWalkModel, for instance) to ALL
          * coordinates, because the results will certainly be erroneous. Use
          * this method only with non-state-aware stochastic models like
          * 'StochasticModel' (constant coordinates) or 'WhiteNoiseModel'.
          */
      virtual SolverPPPGNSS& setCoordinatesModel(StochasticModel* pModel);


         /// Get wet troposphere stochastic model pointer
      virtual StochasticModel* getTroposphereModel(void) const
      { return pTropoStoModel; };


         /** Set zenital wet troposphere stochastic model
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    zenital wet troposphere.
          *
          * \warning Be aware that some stochastic models store their internal
          * state (for instance, 'RandomWalkModel' and 'PhaseAmbiguityModel').
          * If that is your case, you MUST NOT use the SAME model in DIFFERENT
          * solver objects.
          */
      virtual SolverPPPGNSS& setTroposphereModel(StochasticModel* pModel)
      { pTropoStoModel = pModel; return (*this); };


         /// Get receiver clock stochastic model pointer
      virtual StochasticModel* getReceiverClockModel(void) const
      { return pClockStoModel; };


         /** Set receiver clock stochastic model
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    receiver clock.
          *
          * \warning Be aware that some stochastic models store their internal
          * state (for instance, 'RandomWalkModel' and 'PhaseAmbiguityModel').
          * If that is your case, you MUST NOT use the SAME model in DIFFERENT
          * solver objects.
          */
      virtual SolverPPPGNSS& setReceiverClockModel(StochasticModel* pModel)
      { pClockStoModel = pModel; return (*this); };


         /// Get phase biases stochastic model pointer
      virtual StochasticModel* getPhaseBiasesModel(void) const
      { return pBiasStoModel; };


         /** Set phase biases stochastic model.
          *
          * @param pModel      Pointer to StochasticModel associated with
          *                    phase biases.
          *
          * \warning Be aware that some stochastic models store their internal
          * state (for instance, 'RandomWalkModel' and 'PhaseAmbiguityModel').
          * If that is your case, you MUST NOT use the SAME model in DIFFERENT
          * solver objects.
          *
          * \warning This method should be used with caution, because model
          * must be of PhaseAmbiguityModel class in order to make sense.
          */
      virtual SolverPPPGNSS& setPhaseBiasesModel(StochasticModel* pModel)
      { pBiasStoModel = pModel; return (*this); };

         /// get the Gloanss ISB stochastic mdoel.
      virtual StochasticModel* getGLOISBModel (void) const
      { return pISBForGLOStoModel; }

         /** Set Glonass ISB stochastic model.
	 *
	 *\warning: the ISB is stable in a short time, therefore the stochastic 
	 * model can be constant model or RandomWalk model
	 */
      virtual SolverPPPGNSS& setGLOISBModel (StochasticModel* pModel )
      { pISBForGLOStoModel = pModel; return(*this); };

         /// get the Galileo ISB stochastic mdoel.
      virtual StochasticModel* getGALISBModel (void) const
      { return pISBForGALStoModel; }

         /** Set Galileo ISB stochastic model.
	 *
	 *\warning: the ISB is stable in a short time, therefore the stochastic 
	 * model can be constant model or RandomWalk model
	 */
      virtual SolverPPPGNSS& setGALISBModel (StochasticModel* pModel )
      { pISBForGALStoModel = pModel; return(*this); };

         /// get the BeiDou ISB stochastic mdoel.
      virtual StochasticModel* getBDSISBModel (void) const
      { return pISBForBDSStoModel; }

         /** Set BeiDou ISB stochastic model.
	 *
	 *\warning: the ISB is stable in a short time, therefore the stochastic 
	 * model can be constant model or RandomWalk model
	 */
      virtual SolverPPPGNSS& setBDSISBModel (StochasticModel* pModel )
      { pISBForBDSStoModel = pModel; return(*this); };

         /// Get the State Transition Matrix (phiMatrix)
      virtual Matrix<double> getPhiMatrix(void) const
      { return phiMatrix; };

         /** Set the State Transition Matrix (phiMatrix)
          *
          * @param pMatrix     State Transition matrix.
          *
          * \warning Process() methods set phiMatrix and qMatrix according to
          * the stochastic models already defined. Therefore, you must use
          * the Compute() methods directly if you use this method.
          *
          */
      virtual SolverPPPGNSS& setPhiMatrix(const Matrix<double> & pMatrix)
      { phiMatrix = pMatrix; return (*this); };


         /// Get the Noise covariance matrix (QMatrix)
      virtual Matrix<double> getQMatrix(void) const
      { return qMatrix; };


         /** Set the Noise Covariance Matrix (QMatrix)
          *
          * @param pMatrix     Noise Covariance matrix.
          *
          * \warning Process() methods set phiMatrix and qMatrix according to
          * the stochastic models already defined. Therefore, you must use
          * the Compute() methods directly if you use this method.
          *
          */
      virtual SolverPPPGNSS& setQMatrix(const Matrix<double> & pMatrix)
      { qMatrix = pMatrix; return (*this); };


          /** Set the positioning mode, kinematic or static.
           */
      virtual SolverPPPGNSS& setKinematic( bool kinematicMode = true,
                                           double sigmaX = 100.0,
                                           double sigmaY = 100.0,
                                           double sigmaZ = 100.0 );

          /** Set buffer size for convergence statistics. 
           */
      virtual SolverPPPGNSS& setBufferSize(int size )
      { bufferSize = size; return(*this); };

         /** Return the converged flag
          */
      virtual bool getConverged() const
         throw(InvalidRequest);

      virtual std::vector<double> getTTFC() const
      {
          return ttfcVec;
      }


         /// Returns an index identifying this object.
      virtual int getIndex(void) const;


         /// Returns a string identifying this object.
      virtual std::string getClassName(void) const;


         /// Destructor.
      virtual ~SolverPPPGNSS() {};


   private:

         /// Wheather use GPS  
      bool useGPS;

         /// Wheather use Glonass  
      bool useGlonass;

         /// Wheather use BeiDou
      bool useBeiDou;

         /// Wheather use Glonass
      bool useGalileo;

         /// Number of variables
      int numVar;

         /// Number of unknowns
      int numUnknowns;

         /// Number of measurements
      int numMeas;

         /// Weight factor for phase measurements
      double weightFactor;

         /// Boolean flag to indicate reset the solution
      bool resetSol;

         /// Boolean to indicate whether the solution is converged
      bool converged;

         /// Size for convergBuffer
      int bufferSize;

      double startTime;

      std::vector< double > ttfcVec;

         /// Buffer to store the solution drou 
      std::deque<bool> convergBuffer;

         /// Pointer to stochastic model for dx (or dLat) coordinate
      StochasticModel* pCoordXStoModel;

         /// Pointer to stochastic model for dy (or dLon) coordinate
      StochasticModel* pCoordYStoModel;

         /// Pointer to stochastic model for dz (or dH) coordinate
      StochasticModel* pCoordZStoModel;


         /// Pointer to stochastic model for troposphere
      StochasticModel* pTropoStoModel;


         /// Pointer to stochastic model for receiver clock
      StochasticModel* pClockStoModel;
         
	 /// Pointer to stochastic model for inter-system bias for every Glonass satellite,
	 /// include the inter-frequency bias (IFB)
      StochasticModel* pISBForGLOStoModel;

	 /// Pointer to stochastic model for inter-system bias for Galileo
      StochasticModel* pISBForGALStoModel;

	 /// Pointer to stochastic model for inter-system bias for BeiDou
      StochasticModel* pISBForBDSStoModel;


         /// Pointer to stochastic model for phase biases
      StochasticModel* pBiasStoModel;


         /// State Transition Matrix (PhiMatrix)
      Matrix<double> phiMatrix;


         /// Noise covariance matrix (QMatrix)
      Matrix<double> qMatrix;


         /// Geometry matrix
      Matrix<double> hMatrix;


         /// Weights matrix
      Matrix<double> rMatrix;


         /// Measurements vector (Prefit-residuals)
      Vector<double> measVector;


         /// Boolean indicating if this filter was run at least once
      bool firstTime;


         /// Set with all satellites being processed this epoch
      SatIDSet satSet;

         /// Map to store the values of the ambiguity 
      std::map<SatID, double> ambiguityMap;

         /// Map to store the values of the ISB for Glonass satellites
      std::map<SatID, double> GlonassISBMap;

         /// A structure used to store Kalman filter data.
      struct filterData
      {
            // Default constructor initializing the data in the structure
         filterData() {};

         std::map<TypeID, double> vCovMap;    ///< Variables covariance values.
         std::map<SatID,  double> aCovMap;    ///< Ambiguities covariance values.
         std::map<SatID,  double> ISBCovMap;  ///< Glonass ISB covariance values.

      };

         /// Map holding the information regarding every satellite
      std::map<SatID, filterData> ambCovMap;

        /// to store data related to Glonass ISB
      struct ISBData
      {
         ISBData () {};

	 std::map<TypeID, double> ivCovMap;
	 std::map<SatID,  double> iCovMap;
 	 
      };
      std::map<SatID,ISBData> GloISBCovMap;

         /// General Kalman filter object
      SimpleKalmanFilter kFilter;


         /// Initializing method.
      void Init(void);


         /// Constant stochastic model
      StochasticModel constantModel;

         /// White noise stochastic model for position
      WhiteNoiseModel whitenoiseModelX;
      WhiteNoiseModel whitenoiseModelY;
      WhiteNoiseModel whitenoiseModelZ;


         /// Random Walk stochastic model
      RandomWalkModel rwalkModel;
        
      RandomWalkModel rwalkModel2;
      RandomWalkModel rwalkModel3;

         /// White noise stochastic model
      WhiteNoiseModel whitenoiseModel;


         /// Phase biases stochastic model (constant + white noise)
      PhaseAmbiguityModel biasModel;

        /// Glonass ISB stochastic model
      ISBRandomWalkModel  GloISBModel;

         // Some methods that we want to hide
         /// Initial index assigned to this class.
      static int classIndex;

         /// Index belonging to this object.
      int index;

         /// Sets the index and increment classIndex.
      void setIndex(void)
      { index = classIndex++; };


      virtual int Compute( const Vector<double>& prefitResiduals,
                           const Matrix<double>& designMatrix )
         throw(InvalidSolver)
      { return 0; };


      virtual SolverPPPGNSS& setDefaultEqDefinition(
                                       const gnssEquationDefinition& eqDef )
      { return (*this); };



   }; // End of class 'SolverPPPGNSS'

      //@}

}  // End of namespace gpstk

#endif   // GPSTK_SOLVERPPP_HPP