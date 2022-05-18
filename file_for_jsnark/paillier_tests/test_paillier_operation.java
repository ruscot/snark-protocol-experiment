package examples.tests.paillier;

import java.math.BigInteger;

import examples.gadgets.paillier.Paillier_KeyPair;
import examples.gadgets.paillier.Paillier_PublicKey;
import examples.gadgets.paillier.Paillier_keyPairBuilder;

import java.security.Key;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Security;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.prefs.BackingStoreException;

import javax.crypto.Cipher;
import java.security.Signature;

import junit.framework.TestCase;
import org.junit.Assert;
import org.junit.Test;

import circuit.auxiliary.LongElement;
import circuit.config.Config;
import circuit.eval.CircuitEvaluator;
import circuit.structure.CircuitGenerator;
import circuit.structure.Wire;
import junit.framework.TestCase;

import org.junit.Test;

import circuit.structure.WireArray;
import examples.gadgets.rsa.RSAEncryptionV1_5_Gadget;
import examples.gadgets.paillier.Paillier_PrivateKey;
import examples.gadgets.paillier.AdditionLongElementPaillier;
import examples.gadgets.paillier.HornerPolynomialEvalPaillier;
import examples.gadgets.paillier.ModPowLongElementPaillier;
import examples.gadgets.paillier.ModPowPaillier;
import examples.gadgets.paillier.MultiplicationLongElementPaillier;
import examples.gadgets.rsa.PaillierEncryptionAddition_Gadget;


public class test_paillier_operation {
    private Paillier_KeyPair keyPair;
    Paillier_PublicKey publicKey;

    @Test
    public void testEncryption() {
        /***
         * This function test if the ciphered data in paillier's are not equal
         * to the clear data
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger plainData = BigInteger.valueOf(10);

        BigInteger encryptedData = publicKey.encrypt(plainData);

        Assert.assertNotEquals(plainData, encryptedData);
    }

    @Test
    public void testDecyption() {
        /***
         * This function test if the ciphered data in paillier's are equal
         * to the clear data when deciphered
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger plainData = BigInteger.valueOf(10);

        BigInteger encryptedData = publicKey.encrypt(plainData);
        BigInteger decryptedData = keyPair.decrypt(encryptedData);

        Assert.assertEquals(plainData, decryptedData);
    }

    @Test
    public void testHomomorphicAddition() {
        /***
         * This function test the addition in paillier
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger plainA = BigInteger.valueOf(102);
        BigInteger plainB = BigInteger.valueOf(203);

        BigInteger encryptedA = publicKey.encrypt(plainA);
        BigInteger encryptedB = publicKey.encrypt(plainB);

        BigInteger decryptedProduct = keyPair.decrypt(encryptedA.multiply(
                encryptedB).mod(publicKey.getnSquared()));
        BigInteger plainSum = plainA.add(plainB).mod(publicKey.getN());

        Assert.assertEquals(decryptedProduct, plainSum);
    }

    @Test
    public void testHomomorphicConstantMultiplication() {
        /***
         * This function test the mulitplication
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();

        BigInteger plainA = BigInteger.valueOf(14);
        BigInteger plainB = BigInteger.valueOf(203);

        BigInteger encryptedA = publicKey.encrypt(plainA);

        BigInteger decryptedPow = keyPair.decrypt(encryptedA.modPow(plainB,
                publicKey.getnSquared()));
        BigInteger plainSum = plainA.multiply(plainB).mod(publicKey.getN());

        Assert.assertEquals(decryptedPow, plainSum);
    }

    @Test
	public void testPolynomialBigIntPaillier() {
        /***
         * This function test the result of the following calcul :
         * a*((b*(c**x))**x) = a*(b**x)*(c**(x**2)) in paillier 
         * And check wether the result is correct or not 
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger coef0 = BigInteger.valueOf(102);
        BigInteger coef1 = BigInteger.valueOf(203);
        BigInteger coef2 = BigInteger.valueOf(150);

        BigInteger x = BigInteger.valueOf(8);

        BigInteger encryptedCoef0 = publicKey.encrypt(coef0);
        BigInteger encryptedCoef1 = publicKey.encrypt(coef1);
        BigInteger encryptedCoef2 = publicKey.encrypt(coef2);
        BigInteger encryptedX = publicKey.encrypt(x);

        //BigInteger encryptedCoef0WithX = 
        BigInteger res = coef0.add(coef1.multiply(x.pow(1))).add(coef2.multiply(x.pow(2)));
        BigInteger res2 = coef0.add(coef1.add(coef2.multiply(x)).multiply(x));
        System.out.println("Polynomial evaluation res " + res);
        System.out.println("Polynomial evaluation horner res " + res2);
        
        BigInteger paillierModulusValue = publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();
        BigInteger resWithPaillier = encryptedCoef0.multiply(encryptedCoef1.multiply(encryptedCoef2.modPow(x, publicKey.getnSquared())).modPow(x, publicKey.getnSquared())).mod(publicKey.getnSquared());

        BigInteger decryptedPow = keyPair.decrypt(resWithPaillier);
        System.out.println("Decryption with paillier " + decryptedPow);
        Assert.assertEquals(decryptedPow, res);
        Assert.assertEquals(decryptedPow, res2);

	}
}
