package util;

import opt.EvaluationFunction;
import opt.OptimizationAlgorithm;
import shared.FixedIterationTrainer;

public class Profiler {
    public void run(int it, String algoName, EvaluationFunction ef, OptimizationAlgorithm oa) {
        double start, end, trainingTime, accuracy;
        double NANOSEC = 1000000000.0;

        FixedIterationTrainer fit = new FixedIterationTrainer(oa, it);
        start = System.nanoTime();
        fit.train();
        end = System.nanoTime();
        trainingTime = (end - start)/NANOSEC;
        System.out.printf("%s,%d,%.2f,%.2f\n", algoName, it, trainingTime, ef.value(oa.getOptimal()));
    }
}
