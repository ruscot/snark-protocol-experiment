package examples.gadgets.paillier;


import java.math.BigInteger;

public class Paillier_PrivateKey {
    private final BigInteger lambda;
    private final BigInteger preCalculatedDenominator;

    Paillier_PrivateKey(BigInteger lambda, BigInteger preCalculatedDenominator) {
        this.lambda = lambda;

        this.preCalculatedDenominator = preCalculatedDenominator;
    }

    public BigInteger getLambda() {
        return lambda;
    }

    public BigInteger getPreCalculatedDenominator() {
        return preCalculatedDenominator;
    }
}
