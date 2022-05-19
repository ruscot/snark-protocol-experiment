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
    private Integer number_of_coef;
    private Paillier_keyPairBuilder keygen;
    private Paillier_KeyPair keyPair;
    private Paillier_PublicKey publicKey;
    private ArrayList<BigInteger> coeff_in_clear;
    private ArrayList<BigInteger> coeff_in_paillier;
    private BigInteger x;
    private long endTimeCalculation;

    public HornerPolynomialPaillierGenerator(Integer coef_number) {
        /**
         * Generation of paillier keys
        */
        this.keygen = new Paillier_keyPairBuilder();
        this.keyPair = keygen.generateKeyPair();
        this.publicKey = keyPair.getPublicKey();
        this.number_of_coef = coef_number;
        this.coeff_in_clear = new ArrayList<>();
        this.coeff_in_paillier = new ArrayList<>();
        this.x = getRandomBigInteger();
        System.out.println("x value " + this.x);
        this.endTimeCalculation = 0;
    }

    public BigInteger getRandomBigInteger() {
        Random rand = new Random();
        BigInteger result = new BigInteger(253, rand); // (2^253-1) 
        return result;
    }

    public void createCoefficients(){
        for(Integer i = 0; i < this.number_of_coef; i++){
            BigInteger randomCoeff = getRandomBigInteger();
            this.coeff_in_clear.add(randomCoeff);
            BigInteger randomCoeffEncrypted = publicKey.encrypt(randomCoeff);
            this.coeff_in_paillier.add(randomCoeffEncrypted);
        }
    }

    public BigInteger paillierResCiphered(){
        BigInteger res = BigInteger.valueOf(1);
        for(int i = this.coeff_in_paillier.size()-1; i >= 0; i--){
            if(i == 0){
                res = this.coeff_in_paillier.get(i).multiply(res).mod(this.publicKey.getnSquared());
            } else if (i == this.coeff_in_paillier.size() - 1) {
                res = this.coeff_in_paillier.get(i).modPow(this.x, this.publicKey.getnSquared());
            } else {
                res = res.multiply(this.coeff_in_paillier.get(i)).modPow(this.x, this.publicKey.getnSquared());
            }
        }
        return res;
    }

    public long getEndTimeOfCalculation(){
        return this.endTimeCalculation;
    }

    public void genCircuit() throws Exception{
        BigInteger paillierModulusValue = this.publicKey.getnSquared();
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
        generator.writeAllPailliersCoefsWithN(this.coeff_in_paillier, this.x, this.publicKey);
        
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
        

        BigInteger decryptedPow = this.keyPair.decrypt(resWithPaillier);
        BigInteger decryptedPowGadget = this.keyPair.decrypt(t);
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
