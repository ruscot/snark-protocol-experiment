package examples.generators.paillier;

import java.math.BigInteger;
import java.security.PublicKey;
import java.util.ArrayList;
import java.util.Random;

import circuit.auxiliary.LongElement;
import circuit.eval.CircuitEvaluator;
import circuit.structure.CircuitGenerator;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.paillier.HornerPolynomialEvalPaillier;
import examples.gadgets.paillier.Paillier_KeyPair;
import examples.gadgets.paillier.Paillier_PublicKey;
import examples.gadgets.paillier.Paillier_keyPairBuilder;

public class HornerPolynomialPaillierGenerator {
    Integer number_of_coef;
    Paillier_keyPairBuilder keygen;
    Paillier_KeyPair keyPair;
    Paillier_PublicKey publicKey;
    ArrayList<BigInteger> coeff_in_clear;
    ArrayList<BigInteger> coeff_in_paillier;
    BigInteger x;
    private long endTimeCalculation;

    public HornerPolynomialPaillierGenerator(Integer coef_number) {
        /**
         * Generation of paillier keys
        */
        keygen = new Paillier_keyPairBuilder();
        keyPair = keygen.generateKeyPair();
        publicKey = keyPair.getPublicKey();
        this.number_of_coef = coef_number;
        this.coeff_in_clear = new ArrayList<>();
        this.coeff_in_paillier = new ArrayList<>();
        this.x = getRandomBigInteger();
        this.endTimeCalculation = 0;
    }

    public BigInteger getRandomBigInteger() {
        Random rand = new Random();
        BigInteger result = new BigInteger(5, rand); // (2^4-1) = 15 is the maximum value
        return result;
    }

    public void createCoefficients(){
        for(Integer i = 0; i < number_of_coef; i++){
            BigInteger randomCoeff = getRandomBigInteger();
            coeff_in_clear.add(randomCoeff);
            BigInteger randomCoeffEncrypted = publicKey.encrypt(randomCoeff);
            coeff_in_paillier.add(randomCoeffEncrypted);
        }
    }

    public BigInteger paillierResCiphered(){
        BigInteger res = BigInteger.valueOf(1);
        for(int i = coeff_in_paillier.size()-1; i >= 0; i--){
            if(i == 0){
                res = coeff_in_paillier.get(i).multiply(res).mod(publicKey.getnSquared());
            } else if (i == coeff_in_paillier.size() - 1) {
                res = coeff_in_paillier.get(i).modPow(x, publicKey.getnSquared());
            } else {
                res = res.multiply(coeff_in_paillier.get(i)).modPow(x, publicKey.getnSquared());
            }
        }
        return res;
    }

    public long getEndTimeOfCalculation(){
        return this.endTimeCalculation;
    }

    public void genCircuit() throws Exception{
        BigInteger paillierModulusValue = publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();

        CircuitGenerator generator = new CircuitGenerator("Horner_polynomial_eval_big_int_paillier_gadget") {
            ArrayList<Wire[]> inputMessageA;

            Wire[] cipherText;
            LongElement paillierModulus;

            HornerPolynomialEvalPaillier hornerPolynomialEvalPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = new ArrayList<>();
                for(int j = 0; j < coeff_in_paillier.size(); j++){
                    Wire[] coefficientWire = createProverWitnessWireArray(coeff_in_paillier.get(j).toByteArray().length);
                    for(int i = 0; i < coeff_in_paillier.get(j).toByteArray().length; i++){
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
                for(int j = 0; j < coeff_in_paillier.size(); j++){
                    byte[] array = coeff_in_paillier.get(j).toByteArray();
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
        generator.genInputEval();
        generator.prepFilesPaillier();
        generator.writeAllPailliersCoefsWithN(coeff_in_paillier, x, publicKey);
        
        this.endTimeCalculation = System.nanoTime();

        /**
         * Thoose steps are just here to check if our R1CS is correct and if our result is correct too
         * We don't need to take them into the calculation of the time computation of the client
         */
        generator.evalCircuitWithoutInput();
        generator.prepFiles();
        BigInteger resWithPaillier = paillierResCiphered();
        ArrayList<Wire> cipherTextList = generator.getOutWires();
        CircuitEvaluator evaluator = generator.getCircuitEvaluator();
        BigInteger t = BigInteger.ZERO;
        int i = 0;
        for(Wire w:cipherTextList){
            BigInteger val = evaluator.getWireValue(w);
            t = t.add(val.shiftLeft(i*64));
            i++;
        }
        

        BigInteger decryptedPow = keyPair.decrypt(resWithPaillier);
        BigInteger decryptedPowGadget = keyPair.decrypt(t);
        if(!t.equals(resWithPaillier) && decryptedPow.equals(decryptedPowGadget)){
            throw new Exception("Horner evaluation not correct, the result found is not correct...");
        }
    }

    public static void main(String args[]) throws Exception{
        if(args.length < 1){
            System.out.println("You need to give the number of coef for the polynomial");
            return;
        }
        try {
            long startTime = System.nanoTime();
            Integer coef_number = Integer.parseInt(args[0]);
            HornerPolynomialPaillierGenerator hornerPolynomialPaillierGenerator = new HornerPolynomialPaillierGenerator(coef_number);
            hornerPolynomialPaillierGenerator.createCoefficients();
            hornerPolynomialPaillierGenerator.genCircuit();
            long endTime = hornerPolynomialPaillierGenerator.getEndTimeOfCalculation();

            long duration = (endTime - startTime);
            System.out.println("Time of calculation : " + duration/1000000 + " ms");
        } catch(Exception e){
            throw e;
        }
    }
}
