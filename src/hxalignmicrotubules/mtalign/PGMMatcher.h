#pragma once

#include <vector>

#include <mclib/McDArray.h>
#include <mclib/internal/McDMatrix.h>
#include <mclib/McVec2i.h>
#include <mclib/McVec3.h>

#include <dai/alldai.h>
#include <dai/bipgraph.h>
#include <dai/bp.h>
#include <dai/daialg.h>
#include <dai/factor.h>
#include <dai/factorgraph.h>

#include <hxalignmicrotubules/mtalign/Context.h>
#include <hxalignmicrotubules/mtalign/PGMPairWeights.h>

#include <hxalignmicrotubules/api.h>

namespace mtalign {

/// `ConnectedFactorGraph` is used internally by `matchingPGM()` to keep track
/// of the PGM variables and factors used by libdai.
struct ConnectedFactorGraph {
    std::vector<dai::TFactor<dai::Real> > factors;
    McDArray<int> variables;
    McDMatrix<int> adjacencyMatrix;
};

/// `PGMMatcher` is used internally by `matchingPGM()` to implement PGM
/// inference using libdai.
class PGMMatcher {
  public:
    PGMMatcher(const McDArray<McVec3f>& coords1,
               const McDArray<McVec3f>& coords2,
               const McDArray<McVec3f>& directions1,
               const McDArray<McVec3f>& directions2,
               const McDArray<McVec2i>& evidence,
               PGMPairWeights& weightComputationFunction,
               double shiftProbParam);

    /// `setContext()` configures the run-time environment.
    void setContext(Context* ctx);

    void createConnectionFactors(const McDMatrix<int>& adjMat,
                                 const McDArray<int> connectedComps,
                                 std::vector<dai::Factor>& pairFactorList);
    void createSingletonFactors(const McDArray<int>& connectedComp,
                                std::vector<dai::Factor>& singletonFactorList);

    static void
    createAdjacenceMatrix(const McDMatrix<int>& variableAssignmentMat,
                          McDMatrix<int>& adjacenceMatrix);
    static bool getConnectedComponent(
        const McDMatrix<int>& adjacenceMatrix,
        McDMatrix<int>& adjacenceMatrixWithoutConnctedComponent,
        McDArray<int>& connComp, int variableInConnectedComponent = -1);
    void getAllConnectedComponents(std::vector<ConnectedFactorGraph>& graphs);

    void fillSingletonVals(const McDMatrix<int>& variableAssignmentMat,
                           std::vector<dai::Factor>& singletonFactors,
                           ConnectedFactorGraph& graph);
    void getSingletonProbs(const McDArray<int>& assignments, const int varLabel,
                           McDArray<float>& probs);
    void getSingletonProbs(const McDArray<int>& assignments, const int varLabel,
                           McDArray<double>& probs);
    void fillPairVals(const McDMatrix<int>& variableAssignmentMat,
                      std::vector<dai::Factor>& pairFactors,
                      ConnectedFactorGraph& graph);

    void getAssignedValuesForVar(const McDMatrix<float>& allValues,
                                 const McDMatrix<int>& variableAssignmentMat,
                                 const McDArray<int>& possibleAssignemnts,
                                 const int label, const float zeroVal,
                                 McDArray<float>& values);
    void createSameShiftMatrix(const ConnectedFactorGraph& graph,
                               const dai::Factor& curFac,
                               McDMatrix<float>& shiftEntries);

    bool matchPoints(ConnectedFactorGraph& graph, const int numIter,
                     const float maxDiff, const float damping,
                     McDArray<McVec2i>& matchedPointPairs,
                     McDArray<int>& ambiguousAssignments,
                     McDArray<int>& ambiguousAssignmentsMarginalDifference,
                     McDArray<float>& maxMessageDiffsForNodes,
                     McDArray<float>& beliefEntropies);

    void getMaxProbAssignments(const dai::DAIAlgFG* ia,
                               const dai::FactorGraph& fg,
                               const ConnectedFactorGraph& graph,
                               McDArray<McVec2i>& pairs);
    void checkAmbiguities(const dai::DAIAlgFG* ia, const dai::FactorGraph& fg,
                          const ConnectedFactorGraph& graph,
                          McDArray<int>& ambiguities);
    void
    checkAmbiguitiesInAssignments(const ConnectedFactorGraph& graph,
                                  const McDArray<McVec2i>& matchedPointPairs,
                                  McDArray<int>& ambiguities);
    void createFactorGraphFactors(const McDMatrix<int>& variableAssignmentMat,
                                  const McDMatrix<int>& variableAdjacenceMatrix,
                                  ConnectedFactorGraph& factorGraph);

    const McDMatrix<int>& getVariableAssignmentMat() {
        return mVariableAssignmentMat;
    };

    bool getNextConnectedComponent(ConnectedFactorGraph& graph);

    void initConnectedComponentIteration();

    McDArray<McVec3f> mCoords1;
    McDArray<McVec3f> mCoords2;
    McDArray<McVec3f> mDirections1;
    McDArray<McVec3f> mDirections2;
    McDArray<McVec2i> mEvidence;
    McDArray<McVec2i> mQueerEvidence;

    McDMatrix<int> mVariableAssignmentMat;

    McDMatrix<int> mVariableAdjacenceMatrixForConnCompIteration;

    double mShiftProbParam;

    int getEvidenceAssignment(const ConnectedFactorGraph& graph,
                              const int varLabel, int& evidenceIndexInGraph);

    float getMedianZ(const McDArray<McVec3f>& vertices);
    void get3dShiftProbs(const McVec3f pointCoord1, const McVec3f pointCoord2,
                         const McDArray<McVec3f>& assignmentCoordsVar1,
                         const McDArray<McVec3f>& assignmentCoordsVar2,
                         McDMatrix<float>& probs);
    void getShiftDistances(const McVec3f pointCoord1, const McVec3f pointCoord2,
                           const McDArray<McVec3f>& assignmentCoordsVar1,
                           const McDArray<McVec3f>& assignmentCoordsVar2,
                           McDMatrix<float>& distances);
    static float computeShiftDistance(const McVec3f variableCoord1,
                                      const McVec3f variableCoord2,
                                      const McVec3f assignmentCoord1,
                                      const McVec3f assignmentCoord2);

    void outputSingleFactorValues(const ConnectedFactorGraph& graph);
    void outputDoubleFactorValues(const ConnectedFactorGraph& graph);
    void handleEvidenceAssignment(const int varLabel,
                                  const int assignmentInEvidence,
                                  const int assignemtnIndexInWholeModel);

    // computes the index of the entry in coords2 (the possible assignments) for
    // the assigment i for this variable
    int mapVariableAssignmentToIndexInVertexList(const int variableLabel,
                                                 const int assigment);
    int mapIndexInVertexListToVariableAssignment(const int variableLabel,
                                                 const int indexInCoords2Array);
    void getAssignmentsForVariable(const int variableLabel,
                                   McDArray<int>& possibleAssignments);

    mtalign::PGMPairWeights& mWeightComputationFunction;

    void createVariableAssignmentMat();

    void normalize(McDArray<float>& probs);
    void getWeightsForVariable(const McDArray<int>& possibleAssignments,
                               const int label, McDArray<float>& values);

    void getMaxMessageDiffs(dai::DAIAlgFG* infAlgorithm,
                            McDArray<float>& maxMessageDiffsForNodes);

    void getBeliefEntropies(dai::DAIAlgFG* infAlgorithm,
                            McDArray<float>& beliefEntropies);
  private:
    /// `print()` is used for printing via `mContext`.
    void print(QString msg);

    Context* mContext;
};

}  // namespace mtalign
