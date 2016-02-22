package opt.test;

import dist.DiscreteDependencyTree;
import dist.DiscreteUniformDistribution;
import dist.Distribution;
import opt.*;
import opt.example.CountOnesEvaluationFunction;
import opt.ga.*;
import opt.prob.GenericProbabilisticOptimizationProblem;
import opt.prob.MIMIC;
import opt.prob.ProbabilisticOptimizationProblem;
import util.Profiler;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import static java.util.Arrays.asList;

/**
 *
 * @author Andrew Guillory gtg008g@mail.gatech.edu
 * @version 1.0
 */
public class CountOnesTest {
    /** The n value */
    private static final int N = 100;

    public static void main(String[] args) {
        int it = args.length > 0 ? Integer.parseInt(args[0]): 1000;

        int[] ranges = new int[N];
        Arrays.fill(ranges, 10);
        EvaluationFunction ef = new CountOnesEvaluationFunction();
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
        StandardGeneticAlgorithm ga = new StandardGeneticAlgorithm(20, 20, 10, gap);
        MIMIC mimic = new MIMIC(50, 10, pop);

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
