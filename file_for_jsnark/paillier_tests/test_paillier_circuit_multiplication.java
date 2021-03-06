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

public class test_paillier_circuit_multiplication {
    private Paillier_KeyPair keyPair;
    private Paillier_PublicKey publicKey;

    @Test
	public void testMultiplicationBigIntPaillier() {
        /***
         * This function generate a circuit that perform the following calcul :
         * a*x in paillier 
         * And check wether the result is correct or not 
         */
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        this.keyPair = keygen.generateKeyPair();
        this.publicKey = this.keyPair.getPublicKey();
        BigInteger plainA = BigInteger.valueOf(102);
        BigInteger plainB = BigInteger.valueOf(203);

        BigInteger encryptedA = this.publicKey.encrypt(plainA);
        BigInteger encryptedB = this.publicKey.encrypt(plainB);

        BigInteger paillierModulusValue = this.publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();
			
        CircuitGenerator generator = new CircuitGenerator("Multiplication biginteger paillier") {
                Wire[] inputMessageA;
                Wire[] inputMessageB;
                
                Wire[] randomness;
                Wire[] cipherText;
                LongElement paillierModulus;

                MultiplicationLongElementPaillier multiplicationLongElementPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = createProverWitnessWireArray(encryptedA.toByteArray().length); // in bytes
                for(int i = 0; i < encryptedA.toByteArray().length; i++){
                    inputMessageA[i].restrictBitLength(8);
                }
                inputMessageB = createProverWitnessWireArray(encryptedB.toByteArray().length); // in bytes
                for(int i = 0; i < encryptedB.toByteArray().length; i++){
                    inputMessageB[i].restrictBitLength(8);
                }
                multiplicationLongElementPaillier = new MultiplicationLongElementPaillier(paillierModulus, inputMessageA, inputMessageB, paillierModulusSize);
                
                // since randomness is a witness
                //rsaEncryptionV1_5_Gadget.checkRandomnessCompliance();
                Wire[] cipherTextInBytes = multiplicationLongElementPaillier.getOutputWires(); // in bytes
                
                // group every 8 bytes together
                cipherText = new WireArray(cipherTextInBytes).packWordsIntoLargerWords(8, 8);
                makeOutputArray(cipherText,
                        "Output cipher text");
            }

            @Override
            public void generateSampleInput(CircuitEvaluator evaluator) {
                byte[] array = encryptedA.toByteArray();
                
                for(int i = 0; i < array.length; i++){
                    long num = array[array.length - i - 1] & 0xff;
                    evaluator.setWireValue(inputMessageA[i], num);
                }
                array = encryptedB.toByteArray();
                for(int i = 0; i < array.length; i++){
                    long num = array[array.length - i - 1] & 0xff;
                    evaluator.setWireValue(inputMessageB[i], num);
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
        Assert.assertEquals(t, encryptedA.multiply(encryptedB).mod(paillierModulusValue));

	}
}

