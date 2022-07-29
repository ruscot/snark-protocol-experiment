package examples.tests.paillier;

import java.math.BigInteger;
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
import examples.gadgets.paillier.Paillier_KeyPair;
import examples.gadgets.paillier.Paillier_PrivateKey;
import examples.gadgets.paillier.Paillier_PublicKey;
import examples.gadgets.paillier.Paillier_keyPairBuilder;
import examples.gadgets.paillier.AdditionLongElementPaillier;
import examples.gadgets.paillier.HornerPolynomialEvalPaillier;
import examples.gadgets.paillier.ModPowLongElementPaillier;
import examples.gadgets.paillier.ModPowPaillier;
import examples.gadgets.paillier.MultiplicationLongElementPaillier;
import examples.gadgets.rsa.PaillierEncryptionAddition_Gadget;

public class test_paillier_horner_polynomial_eval_big_integer {
    private Paillier_KeyPair keyPair;
    private Paillier_PublicKey publicKey;

    @Test
	public void testHornerPolynomialEvalBigIntPaillierGadget() {
        /***
         * This function generate a circuit that perform the following calcul :
         * a*((b*(c**x))**x) = a*(b**x)*(c**(x**2)) in paillier 
         * And check wether the result is correct or not 
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        this.keyPair = keygen.generateKeyPair();
        this.publicKey = this.keyPair.getPublicKey();
        BigInteger coef0 = BigInteger.valueOf(102);
        BigInteger coef1 = BigInteger.valueOf(203);
        BigInteger coef2 = BigInteger.valueOf(150);

        BigInteger x = BigInteger.valueOf(8);

        BigInteger encryptedCoef0 = this.publicKey.encrypt(coef0);
        BigInteger encryptedCoef1 = this.publicKey.encrypt(coef1);
        BigInteger encryptedCoef2 = this.publicKey.encrypt(coef2);
        ArrayList<BigInteger> cipheredCoefficients = new ArrayList<>();
        cipheredCoefficients.add(encryptedCoef0);
        cipheredCoefficients.add(encryptedCoef1);
        cipheredCoefficients.add(encryptedCoef2);
        //BigInteger encryptedCoef0WithX = 
        BigInteger res = coef0.add(coef1.multiply(x.pow(1))).add(coef2.multiply(x.pow(2)));
        BigInteger res2 = coef0.add(coef1.add(coef2.multiply(x)).multiply(x));
        System.out.println("Polynomial evaluation res " + res);
        System.out.println("Polynomial evaluation horner res " + res2);
        
        BigInteger paillierModulusValue = this.publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();
        BigInteger resWithPaillier = encryptedCoef0.multiply(encryptedCoef1.multiply(encryptedCoef2.modPow(
                    x, this.publicKey.getnSquared())).modPow(x, this.publicKey.getnSquared())).mod(this.publicKey.getnSquared());


        CircuitGenerator generator = new CircuitGenerator("Horner polynomial eval big int paillier gadget") {
            ArrayList<Wire[]> inputMessageA;
            
            Wire[] cipherText;
            LongElement paillierModulus;

            HornerPolynomialEvalPaillier hornerPolynomialEvalPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = new ArrayList<>();
                for(int j = 0; j < cipheredCoefficients.size(); j++){
                    Wire[] coefficientWire = createProverWitnessWireArray(cipheredCoefficients.get(j).toByteArray().length);
                    for(int i = 0; i < cipheredCoefficients.get(j).toByteArray().length; i++){
                        coefficientWire[i].restrictBitLength(8);
                    }
                    inputMessageA.add(coefficientWire);
                }
                
                
                hornerPolynomialEvalPaillier = new HornerPolynomialEvalPaillier(paillierModulus, inputMessageA, x, paillierModulusSize);
                
                Wire[] cipherTextInBytes = hornerPolynomialEvalPaillier.getOutputWires(); // in bytes
                
                // group every 8 bytes together
                cipherText = new WireArray(cipherTextInBytes).packWordsIntoLargerWords(8, 8);
                makeOutputArray(cipherText,
                        "Output cipher text");
            }

            @Override
            public void generateSampleInput(CircuitEvaluator evaluator) {
                for(int j = 0; j < cipheredCoefficients.size(); j++){
                    byte[] array = cipheredCoefficients.get(j).toByteArray();
                    for(int i = 0; i < array.length; i++){
                        long num = array[array.length - i - 1] & 0xff;
                        evaluator.setWireValue(inputMessageA.get(j)[i], num);
                    }
                }
                evaluator.setWireValue(this.paillierModulus, paillierModulusValue,
                            LongElement.CHUNK_BITWIDTH);
            }
        };

        generator.generateCircuit();
        generator.evalCircuit();
        CircuitEvaluator evaluator = generator.getCircuitEvaluator();
        ArrayList<Wire> cipherTextList = generator.getOutWires();
        BigInteger t = BigInteger.ZERO;
        int i = 0;
        for(Wire w:cipherTextList){
            BigInteger val = evaluator.getWireValue(w);
            t = t.add(val.shiftLeft(i*64));
            i++;
        }
        

        BigInteger decryptedPow = this.keyPair.decrypt(resWithPaillier);
        Assert.assertEquals(t, resWithPaillier);
        BigInteger decryptedPowGadget = this.keyPair.decrypt(t);
        Assert.assertEquals(decryptedPow, decryptedPowGadget);
	}
}