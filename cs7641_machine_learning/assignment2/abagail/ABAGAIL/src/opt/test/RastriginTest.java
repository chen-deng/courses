package opt.test;

import dist.DiscreteDependencyTree;
import dist.DiscreteUniformDistribution;
import dist.Distribution;
import opt.*;
import opt.example.RastriginEvaluationFunction;
import opt.ga.*;
import opt.prob.GenericProbabilisticOptimizationProblem;
import opt.prob.MIMIC;
import opt.prob.ProbabilisticOptimizationProblem;
import util.Profiler;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import static java.util.Arrays.asList;

public class RastriginTest {
    /** The n value */
    private static final int N = 100;

    public static void main(String[] args) {
        int it = args.length > 0 ? Integer.parseInt(args[0]): 1000;

        int[] ranges = new int[N];
        Arrays.fill(ranges, 5);

        EvaluationFunction ef = new RastriginEvaluationFunction();
        Distribution odd = new DiscreteUniformDistribution(ranges);
        NeighborFunction nf = new DiscreteChangeOneNeighbor(ranges);
        MutationFunction mf = new DiscreteChangeOneMutation(ranges);
//        CrossoverFunction cf = new SingleCrossOver();
        CrossoverFunction cf = new UniformCrossOver();
        Distribution df = new DiscreteDependencyTree(.1, ranges);
        HillClimbingProblem hcp = new GenericHillClimbingProblem(ef, odd, nf);
        GeneticAlgorithmProblem gap = new GenericGeneticAlgorithmProblem(ef, odd, mf, cf);
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
