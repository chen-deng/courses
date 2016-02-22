package opt.test;

import dist.DiscreteDependencyTree;
import dist.DiscreteUniformDistribution;
import dist.Distribution;
import opt.*;
import opt.example.KnapsackEvaluationFunction;
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
 * A test of the knap sack problem
 * @author Andrew Guillory gtg008g@mail.gatech.edu
 * @version 1.0
 */
public class KnapsackTest {
    /** Random number generator */
    private static final Random random = new Random();
    /** The number of items */
    private static final int NUM_ITEMS = 40;
    /** The number of copies each */
    private static final int COPIES_EACH = 4;
    /** The maximum weight for a single element */
    private static final double MAX_WEIGHT = 50;
    /** The maximum volume for a single element */
    private static final double MAX_VOLUME = 50;
    /** The volume of the knapsack */
    private static final double KNAPSACK_VOLUME =
         MAX_VOLUME * NUM_ITEMS * COPIES_EACH * .4;
    /**
     * The test main
     * @param args ignored
     */
    public static void main(String[] args) {
        int it = args.length > 0 ? Integer.parseInt(args[0]): 1000;
        int[] copies = new int[NUM_ITEMS];
        Arrays.fill(copies, COPIES_EACH);
        double[] weights = new double[NUM_ITEMS];
        double[] volumes = new double[NUM_ITEMS];
        for (int i = 0; i < NUM_ITEMS; i++) {
            weights[i] = random.nextDouble() * MAX_WEIGHT;
            volumes[i] = random.nextDouble() * MAX_VOLUME;
        }
         int[] ranges = new int[NUM_ITEMS];
        Arrays.fill(ranges, COPIES_EACH + 1);
        EvaluationFunction ef = new KnapsackEvaluationFunction(weights, volumes, KNAPSACK_VOLUME, copies);
        Distribution odd = new DiscreteUniformDistribution(ranges);
        NeighborFunction nf = new DiscreteChangeOneNeighbor(ranges);
        MutationFunction mf = new DiscreteChangeOneMutation(ranges);
        CrossoverFunction cf = new UniformCrossOver();
        Distribution df = new DiscreteDependencyTree(.1, ranges);
        HillClimbingProblem hcp = new GenericHillClimbingProblem(ef, odd, nf);
        GeneticAlgorithmProblem gap = new GenericGeneticAlgorithmProblem(ef, odd, mf, cf);
        ProbabilisticOptimizationProblem pop = new GenericProbabilisticOptimizationProblem(ef, odd, df);

        RandomizedHillClimbing rhc = new RandomizedHillClimbing(hcp);

        SimulatedAnnealing sa = new SimulatedAnnealing(100, .95, hcp);

        StandardGeneticAlgorithm ga = new StandardGeneticAlgorithm(200, 150, 25, gap);

        MIMIC mimic = new MIMIC(200, 100, pop);

        HashMap<String, OptimizationAlgorithm> algos = new HashMap<String, OptimizationAlgorithm>();
        algos.put("RHC", rhc);
        algos.put("SA", sa);
        algos.put("GA", ga);
        algos.put("MIMIC", mimic);

        List<String> algoNames = asList("SA", "GA", "MIMIC");

        Profiler profiler = new Profiler();

        for (String algoName : algoNames) {
            profiler.run(it, algoName, ef, algos.get(algoName));
        }
    }
}
