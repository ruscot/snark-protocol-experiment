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

public class test_paillier_circuit_addition {
    private Paillier_KeyPair keyPair;
    Paillier_PublicKey publicKey;

    @Test
	public void testAdditionBigIntPaillier() {
        //1 -> 7174
        //2 -> 6900
        //4 -> 6799
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger plainA = BigInteger.valueOf(102);
        BigInteger plainB = BigInteger.valueOf(203);

        BigInteger encryptedA = publicKey.encrypt(plainA);
        BigInteger encryptedB = publicKey.encrypt(plainB);

        BigInteger paillierModulusValue = publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();
			
        CircuitGenerator generator = new CircuitGenerator("Addition biginteger paillier") {
                Wire[] inputMessageA;
                Wire[] inputMessageB;
                
                Wire[] randomness;
                Wire[] cipherText;
                LongElement paillierModulus;

                AdditionLongElementPaillier additionLongElementPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = createProverWitnessWireArray((int)Math.ceil(encryptedA.toByteArray().length/4)); // in bytes
                for(int i = 0; i < (int)Math.ceil(encryptedA.toByteArray().length/4); i++){
                    inputMessageA[i].restrictBitLength(32);
                }
                inputMessageB = createProverWitnessWireArray((int)Math.ceil(encryptedB.toByteArray().length/4)); // in bytes
                for(int i = 0; i < (int)Math.ceil(encryptedB.toByteArray().length/4); i++){
                    inputMessageB[i].restrictBitLength(32);
                }
                additionLongElementPaillier = new AdditionLongElementPaillier(paillierModulus, inputMessageA, inputMessageB, paillierModulusSize);
                
                // since randomness is a witness
                //rsaEncryptionV1_5_Gadget.checkRandomnessCompliance();
                Wire[] cipherTextInBytes = additionLongElementPaillier.getOutputWires(); // in bytes
                
                // group every 8 bytes together
                cipherText = new WireArray(cipherTextInBytes).packWordsIntoLargerWords(8, 8);
                makeOutputArray(cipherText,
                        "Output cipher text");
            }

            @Override
            public void generateSampleInput(CircuitEvaluator evaluator) {
                byte[] array = encryptedA.toByteArray();
                for(int i = 0; i < (int)Math.ceil(array.length/4); i++){
                    long num = array[array.length - i*4 - 1] & 0xff;
                    if(array.length - (i*4+3) - 1 < 0){
                        System.out.println("Here for A");
                        evaluator.setWireValue(inputMessageA[i], num);
                    } else {
                        long num2 = array[array.length - (i*4+1) - 1] & 0xff;
                        long num3 = array[array.length - (i*4+2) - 1] & 0xff;
                        long num4 = array[array.length - (i*4+3) - 1] & 0xff;
                        evaluator.setWireValue(inputMessageA[i], (num4<<24) + (num3<<16) + (num2<<8) + num);
                    }
                    //evaluator.setWireValue(inputMessageA[i], 1);
                }
                array = encryptedB.toByteArray();
                for(int i = 0; i < (int)Math.ceil(array.length/4); i++){
                    long num = array[array.length - i*4 - 1] & 0xff;
                    if(array.length - (i*4+3) - 1 < 0){
                        System.out.println("Here for B");
                        evaluator.setWireValue(inputMessageB[i], num);
                    } else {
                        long num2 = array[array.length - (i*4+1) - 1] & 0xff;
                        long num3 = array[array.length - (i*4+2) - 1] & 0xff;
                        long num4 = array[array.length - (i*4+3) - 1] & 0xff;
                        evaluator.setWireValue(inputMessageB[i], (num4<<24) + (num3<<16) + (num2<<8) + num);
                    }
                    //evaluator.setWireValue(inputMessageA[i], 1);
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
        Assert.assertEquals(t, encryptedA.add(encryptedB).mod(paillierModulusValue));

	}
}
