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

public class test_paillier_circuit_mod_pow {
    private Paillier_KeyPair keyPair;
    private Paillier_PublicKey publicKey;

    @Test
	public void testModPowPaillier() {
        /***
         * This function generate a circuit that perform the following calcul :
         * a**x in paillier 
         * With x a little integer
         * And check wether the result is correct or not 
         */
        //1 -> 44284
        //2 -> 44156
        //4 -> 44090
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        this.keyPair = keygen.generateKeyPair();
        this.publicKey = this.keyPair.getPublicKey();
        BigInteger coef0 = BigInteger.valueOf(102);

        BigInteger bigIntX = BigInteger.valueOf(8);
        int x = 8;

        BigInteger encryptedCoef0 = this.publicKey.encrypt(coef0);
        
        BigInteger paillierModulusValue = this.publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();

        CircuitGenerator generator = new CircuitGenerator("Mod pow long integer paillier") {
            Wire[] inputMessageA;
            
            Wire[] randomness;
            Wire[] cipherText;
            LongElement paillierModulus;

            ModPowPaillier modPowPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = createProverWitnessWireArray((int)Math.ceil(encryptedCoef0.toByteArray().length/4)); // in bytes
                for(int i = 0; i < (int)Math.ceil(encryptedCoef0.toByteArray().length/4); i++){
                    inputMessageA[i].restrictBitLength(32);
                }
                
                modPowPaillier = new ModPowPaillier(paillierModulus, inputMessageA, x, paillierModulusSize);
                
                // since randomness is a witness
                //rsaEncryptionV1_5_Gadget.checkRandomnessCompliance();
                Wire[] cipherTextInBytes = modPowPaillier.getOutputWires(); // in bytes
                
                // group every 8 bytes together
                cipherText = new WireArray(cipherTextInBytes).packWordsIntoLargerWords(8, 8);
                makeOutputArray(cipherText,
                        "Output cipher text");
            }

            @Override
            public void generateSampleInput(CircuitEvaluator evaluator) {

                byte[] array = encryptedCoef0.toByteArray();
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

        BigInteger res = encryptedCoef0.modPow(bigIntX, this.publicKey.getnSquared());
        Assert.assertEquals(t, res);

	}
}
