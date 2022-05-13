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

public class test_paillier {
    private Paillier_KeyPair keyPair;
    Paillier_PublicKey publicKey;

    @Test
    public void testEncryption() {
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger plainData = BigInteger.valueOf(10);

        BigInteger encryptedData = publicKey.encrypt(plainData);

        Assert.assertNotEquals(plainData, encryptedData);
    }

    @Test
    public void testDecyption() {
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
        //Addition
        //encryptedA.multiply(encryptedB).mod(publicKey.getnSquared()));
        //Multiplication
        //encryptedCoef0.modPow(plainB,publicKey.getnSquared());
        BigInteger resWithPaillier = encryptedCoef0.multiply(encryptedCoef1.multiply(encryptedCoef2.modPow(x, publicKey.getnSquared())).modPow(x, publicKey.getnSquared())).mod(publicKey.getnSquared());

        BigInteger decryptedPow = keyPair.decrypt(resWithPaillier);
        System.out.println("Decryption with paillier " + decryptedPow);

	}

    @Test
	public void testMultiplicationBigIntPaillier() {
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger plainA = BigInteger.valueOf(102);
        BigInteger plainB = BigInteger.valueOf(203);

        BigInteger encryptedA = publicKey.encrypt(plainA);
        BigInteger encryptedB = publicKey.encrypt(plainB);

        BigInteger paillierModulusValue = publicKey.getnSquared();
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
    
    @Test
	public void testAdditionBigIntPaillier() {
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
                inputMessageA = createProverWitnessWireArray(encryptedA.toByteArray().length); // in bytes
                for(int i = 0; i < encryptedA.toByteArray().length; i++){
                    inputMessageA[i].restrictBitLength(8);
                }
                inputMessageB = createProverWitnessWireArray(encryptedB.toByteArray().length); // in bytes
                for(int i = 0; i < encryptedB.toByteArray().length; i++){
                    inputMessageB[i].restrictBitLength(8);
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
        Assert.assertEquals(t, encryptedA.add(encryptedB).mod(paillierModulusValue));

	}

    @Test
	public void testModPowPaillier() {
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger coef0 = BigInteger.valueOf(102);

        BigInteger bigIntX = BigInteger.valueOf(8);
        int x = 8;

        BigInteger encryptedCoef0 = publicKey.encrypt(coef0);
        
        BigInteger paillierModulusValue = publicKey.getnSquared();
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
                inputMessageA = createProverWitnessWireArray(encryptedCoef0.toByteArray().length); // in bytes
                for(int i = 0; i < encryptedCoef0.toByteArray().length; i++){
                    inputMessageA[i].restrictBitLength(8);
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
                
                for(int i = 0; i < array.length; i++){
                    long num = array[array.length - i - 1] & 0xff;
                    evaluator.setWireValue(inputMessageA[i], num);
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

        BigInteger res = encryptedCoef0.modPow(bigIntX, publicKey.getnSquared());
        Assert.assertEquals(t, res);

	}

    @Test
	public void testModPowBigIntPaillier() {
        Paillier_keyPairBuilder keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        BigInteger coef0 = BigInteger.valueOf(102);

        BigInteger x = BigInteger.valueOf(8);

        BigInteger encryptedCoef0 = publicKey.encrypt(coef0);
        
        BigInteger paillierModulusValue = publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();

        CircuitGenerator generator = new CircuitGenerator("Mod pow long integer paillier") {
            Wire[] inputMessageA;
            
            Wire[] randomness;
            Wire[] cipherText;
            LongElement paillierModulus;

            ModPowLongElementPaillier modPowLongElementPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = createProverWitnessWireArray(encryptedCoef0.toByteArray().length); // in bytes
                for(int i = 0; i < encryptedCoef0.toByteArray().length; i++){
                    inputMessageA[i].restrictBitLength(8);
                }
                
                modPowLongElementPaillier = new ModPowLongElementPaillier(paillierModulus, inputMessageA, x, paillierModulusSize);
                
                // since randomness is a witness
                //rsaEncryptionV1_5_Gadget.checkRandomnessCompliance();
                Wire[] cipherTextInBytes = modPowLongElementPaillier.getOutputWires(); // in bytes
                
                // group every 8 bytes together
                cipherText = new WireArray(cipherTextInBytes).packWordsIntoLargerWords(8, 8);
                makeOutputArray(cipherText,
                        "Output cipher text");
            }

            @Override
            public void generateSampleInput(CircuitEvaluator evaluator) {
                byte[] array = encryptedCoef0.toByteArray();
                
                for(int i = 0; i < array.length; i++){
                    long num = array[array.length - i - 1] & 0xff;
                    evaluator.setWireValue(inputMessageA[i], num);
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

        BigInteger res = encryptedCoef0.modPow(x, publicKey.getnSquared());
        Assert.assertEquals(t, res);

	}

    @Test
	public void testHornerPolynomialEvalBigIntPaillierGadget() {
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
        ArrayList<BigInteger> cipheredCoefficients = new ArrayList<>();
        cipheredCoefficients.add(encryptedCoef0);
        cipheredCoefficients.add(encryptedCoef1);
        cipheredCoefficients.add(encryptedCoef2);
        //BigInteger encryptedCoef0WithX = 
        BigInteger res = coef0.add(coef1.multiply(x.pow(1))).add(coef2.multiply(x.pow(2)));
        BigInteger res2 = coef0.add(coef1.add(coef2.multiply(x)).multiply(x));
        System.out.println("Polynomial evaluation res " + res);
        System.out.println("Polynomial evaluation horner res " + res2);
        
        BigInteger paillierModulusValue = publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();
        //Addition
        //encryptedA.multiply(encryptedB).mod(publicKey.getnSquared()));
        //Multiplication
        //encryptedCoef0.modPow(plainB,publicKey.getnSquared());
        BigInteger resWithPaillier = encryptedCoef0.multiply(encryptedCoef1.multiply(encryptedCoef2.modPow(x, publicKey.getnSquared())).modPow(x, publicKey.getnSquared())).mod(publicKey.getnSquared());


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
                
                // since randomness is a witness
                //rsaEncryptionV1_5_Gadget.checkRandomnessCompliance();
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
        generator.prepFiles();
        generator.runLibsnark();
        ArrayList<Wire> cipherTextList = generator.getOutWires();
        BigInteger t = BigInteger.ZERO;
        int i = 0;
        for(Wire w:cipherTextList){
            BigInteger val = evaluator.getWireValue(w);
            t = t.add(val.shiftLeft(i*64));
            i++;
        }
        

        BigInteger decryptedPow = keyPair.decrypt(resWithPaillier);
        Assert.assertEquals(t, resWithPaillier);
        BigInteger decryptedPowGadget = keyPair.decrypt(t);
        Assert.assertEquals(decryptedPow, decryptedPowGadget);
	}
}