package examples.gadgets.paillier;

import java.math.BigInteger;
public class Paillier_KeyPair {
    private final Paillier_PrivateKey privateKey;
    private final Paillier_PublicKey publicKey;
    private final BigInteger upperBound;

    Paillier_KeyPair(Paillier_PrivateKey privateKey, Paillier_PublicKey publicKey, BigInteger upperBound) {
        this.privateKey = privateKey;
        this.publicKey = publicKey;
        this.upperBound = upperBound;
    }

    public Paillier_PrivateKey getPrivateKey() {
        return privateKey;
    }

    public Paillier_PublicKey getPublicKey() {
        return publicKey;
    }

    /**
     * Decrypts the given ciphertext.
     *
     * @param c The ciphertext that should be decrypted.
     * @return The corresponding plaintext. If an upper bound was given to {@link KeyPairBuilder},
     * the result can also be negative. See {@link KeyPairBuilder#upperBound(BigInteger)} for details.
     */
    public final BigInteger decrypt(BigInteger c) {

        BigInteger n = publicKey.getN();
        BigInteger nSquare = publicKey.getnSquared();
        BigInteger lambda = privateKey.getLambda();

        BigInteger u = privateKey.getPreCalculatedDenominator();

        BigInteger p = c.modPow(lambda, nSquare).subtract(BigInteger.ONE).divide(n).multiply(u).mod(n);

        if (upperBound != null && p.compareTo(upperBound) > 0) {
            p = p.subtract(n);
        }

        return p;
    }
}
