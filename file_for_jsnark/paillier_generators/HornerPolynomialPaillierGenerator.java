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
    }

    public BigInteger getRandomBigInteger() {
        Random rand = new Random();
        BigInteger result = new BigInteger(5, rand); // (2^4-1) = 15 is the maximum value
        //BigInteger result = new BigInteger(this.publicKey.getN().bitLength()-1, rand); // (2^4-1) = 15 is the maximum value
        return result;
    }

    public void createCoefficients(){
        BigInteger coef0 = BigInteger.valueOf(2);
        BigInteger coef1 = BigInteger.valueOf(3);
        this.x = BigInteger.valueOf(2);

        /*BigInteger encryptedCoef0 = publicKey.encrypt(coef0);
        BigInteger encryptedCoef1 = publicKey.encrypt(coef1);*/
        BigInteger encryptedCoef0 = new BigInteger("6295391305699821800384282937566923011915353854256799621207499486153794316019694410610593715857021461747696402790856087287432623375694099962007685022936057022696292424032907201903695435587934575991212245241185843954277937518528543673802157904116293497986943277675416940620524292693141093032777554632355528186716563901923593163440672394333507364177221685588719801348534679957782205357497646337110872998592999529236792134723499709346134510290323521146365621174980210259232882063270868582314180690337935092754361137298894959873392569859644839349439244883756117905060168073250304699088689735130352014673926671777214517392");
        BigInteger encryptedCoef1 = new BigInteger("4337793653751519365928649665079040707370918324221037141856086840427538160937831338313811878647783136656039684513066935317391027354071798188184005331603884169566106668041382980700485034604575353678999014623009370853215704582049326641325993555590448554048646303267537725120071082924714645354891669593864638255726361531810214799814954282191508836820303974301980212876289957073183821874189006299259837411364714140908531342937251872387822459174741882077987912989564503434381357396669688409022601238596347230739918000855435721870817803593157970074476961851710385107741223056132196400365574944838833070895759325749412474536");
        
        coeff_in_paillier.add(encryptedCoef0);
        coeff_in_paillier.add(encryptedCoef1);
        System.out.println(encryptedCoef1);
        System.out.println(encryptedCoef0);
        
        /*for(Integer i = 0; i < number_of_coef; i++){
            BigInteger randomCoeff = getRandomBigInteger();
            coeff_in_clear.add(randomCoeff);
            BigInteger randomCoeffEncrypted = publicKey.encrypt(randomCoeff);
            coeff_in_paillier.add(randomCoeffEncrypted);
        }*/
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

    public void genCircuit(){
        BigInteger paillierModulusValue = publicKey.getnSquared();
        int paillierModulusSize = paillierModulusValue.bitLength();
        //Addition
        //encryptedA.multiply(encryptedB).mod(publicKey.getnSquared()));
        //Multiplication
        //encryptedCoef0.modPow(plainB,publicKey.getnSquared());
        //BigInteger resWithPaillier = paillierResCiphered();


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
        generator.evalCircuit();
        //generator.genInputEval();
        //CircuitEvaluator evaluator = generator.getCircuitEvaluator();
        //generator.prepFilesPaillier();
        
        
        //generator.evalCircuitWithoutInput();
        generator.prepFiles();
        //generator.circuitFromFile();
        //generator.printCircuit();
        //generator.prepFiles();
        /*ArrayList<Wire> cipherTextList = generator.getOutWires();
        BigInteger t = BigInteger.ZERO;
        int i = 0;
        for(Wire w:cipherTextList){
            BigInteger val = evaluator.getWireValue(w);
            t = t.add(val.shiftLeft(i*64));
            i++;
        }
        

        BigInteger decryptedPow = keyPair.decrypt(resWithPaillier);
        BigInteger decryptedPowGadget = keyPair.decrypt(t);
        if(t.equals(resWithPaillier) && decryptedPow.equals(decryptedPowGadget)){
            System.out.println("All fine");
        } else {
            System.out.println("Problem !");
        }*/
    }

    public static void main(String args[]){
        if(args.length < 1){
            System.out.println("You need to give the number of coef for the polynomial");
            return;
        }
        try {
            Integer coef_number = Integer.parseInt(args[0]);
            System.out.println(coef_number);
            HornerPolynomialPaillierGenerator hornerPolynomialPaillierGenerator = new HornerPolynomialPaillierGenerator(coef_number);
            hornerPolynomialPaillierGenerator.createCoefficients();
            hornerPolynomialPaillierGenerator.genCircuit();
        } catch(Exception e){
            System.out.println("You didn't give an integer for the number of coefficient");
            return;
        }
    }
}
