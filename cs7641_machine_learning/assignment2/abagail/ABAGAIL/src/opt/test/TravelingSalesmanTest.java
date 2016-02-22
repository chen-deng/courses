package opt.test;

import dist.DiscreteDependencyTree;
import dist.DiscretePermutationDistribution;
import dist.DiscreteUniformDistribution;
import dist.Distribution;
import opt.*;
import opt.example.TravelingSalesmanCrossOver;
import opt.example.TravelingSalesmanEvaluationFunction;
import opt.example.TravelingSalesmanRouteEvaluationFunction;
import opt.example.TravelingSalesmanSortEvaluationFunction;
import opt.ga.*;
import opt.prob.GenericProbabilisticOptimizationProblem;
import opt.prob.MIMIC;
import opt.prob.ProbabilisticOptimizationProblem;
import util.Profiler;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Random;

import static java.util.Arrays.asList;

/**
 *
 * @author Andrew Guillory gtg008g@mail.gatech.edu
 * @version 1.0
 */
public class TravelingSalesmanTest {
    /** The n value */
    private static final int N = 50;
    /**
     * The test main
     * @param args ignored
     */
    public static void main(String[] args) {
        int it = args.length > 0 ? Integer.parseInt(args[0]) : 1000;
        Random random = new Random();
        // create the random points
        double[][] points = new double[N][2];
        for (int i = 0; i < points.length; i++) {
            points[i][0] = random.nextDouble();
            points[i][1] = random.nextDouble();
        }
        // for rhc, sa, and ga we use a permutation based encoding
        TravelingSalesmanEvaluationFunction ef = new TravelingSalesmanRouteEvaluationFunction(points);
        Distribution odd = new DiscretePermutationDistribution(N);
        NeighborFunction nf = new SwapNeighbor();
        MutationFunction mf = new SwapMutation();
        CrossoverFunction cf = new TravelingSalesmanCrossOver(ef);
        HillClimbingProblem hcp = new GenericHillClimbingProblem(ef, odd, nf);
        GeneticAlgorithmProblem gap = new GenericGeneticAlgorithmProblem(ef, odd, mf, cf);

        RandomizedHillClimbing rhc = new RandomizedHillClimbing(hcp);

        SimulatedAnnealing sa = new SimulatedAnnealing(1E12, .95, hcp);

        StandardGeneticAlgorithm ga = new StandardGeneticAlgorithm(200, 150, 20, gap);

        // for mimic we use a sort encoding
        ef = new TravelingSalesmanSortEvaluationFunction(points);
        int[] ranges = new int[N];
        Arrays.fill(ranges, N);
        odd = new DiscreteUniformDistribution(ranges);
        Distribution df = new DiscreteDependencyTree(.1, ranges);
        ProbabilisticOptimizationProblem pop = new GenericProbabilisticOptimizationProblem(ef, odd, df);

        HashMap<String, OptimizationAlgorithm> algos = new HashMap<String, OptimizationAlgorithm>();
        algos.put("RHC", new RandomizedHillClimbing(hcp));
        algos.put("SA", new SimulatedAnnealing(1E11, .95, hcp));
        algos.put("GA", new StandardGeneticAlgorithm(200, 100, 1, gap));
        algos.put("MIMIC", new MIMIC(200, 20, pop));

        List<String> algoNames = asList("RHC", "SA", "GA", "MIMIC");
        Profiler profiler = new Profiler();

        for (String algoName : algoNames) {
            profiler.run(it, algoName, ef, algos.get(algoName));
        }
    }
}
