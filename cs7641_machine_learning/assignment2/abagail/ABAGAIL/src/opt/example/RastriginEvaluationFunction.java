package opt.example;

import opt.EvaluationFunction;
import shared.Instance;
import util.linalg.Vector;

/**
 * Created by rkekatpure on 10/18/15.
 */
public class RastriginEvaluationFunction implements EvaluationFunction {

    /**
     * @see opt.EvaluationFunction#value(opt.OptimizationData)
     */
    public double value(Instance d) {
        Vector data = d.getData();
        double A = 10, x;
        long N = data.size();

        // Implement Rastrigin function
        double val = 0;
        for (int i = 0; i < N; ++i) {
            x = data.get(i);
            val += x * x - A * Math.cos(2 * Math.PI * x);
        }
        return  100 - (A * N + val);
    }
}
