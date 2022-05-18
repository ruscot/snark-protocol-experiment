package examples.generators.paillier;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.math.BigInteger;
import java.util.ArrayList;

import circuit.auxiliary.LongElement;
import circuit.eval.CircuitEvaluator;
import circuit.structure.CircuitGenerator;
import circuit.structure.Wire;
import circuit.structure.WireArray;
import examples.gadgets.paillier.HornerPolynomialEvalPaillier;

public class HornerPolynomialPaillierFromFileGenerator {
    private ArrayList<BigInteger> coeff_in_paillier;
    private BigInteger x;
    private BigInteger nSquared;

    public HornerPolynomialPaillierFromFileGenerator() {
        this.coeff_in_paillier = new ArrayList<>();
        readAllPailliersCoefsWithN();
    }

    public void readAllPailliersCoefsWithN() {
        try {
            BufferedReader reader;
            reader = new BufferedReader(new FileReader(
                    "AllPailliersCoeffAndInformation.out"));
            String line = reader.readLine();
            int i = 0;
            while (line != null) {
                if(i == 0){
                    this.nSquared = new BigInteger(line);
                } else if (i == 1){
                    this.x = new BigInteger(line);
                } else {
                    this.coeff_in_paillier.add(new BigInteger(line));
                }
                
                line = reader.readLine();
                i+=1;
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public BigInteger paillierResCiphered(){
        BigInteger res = BigInteger.valueOf(1);
        for(int i = this.coeff_in_paillier.size()-1; i >= 0; i--){
            if(i == 0){
                res = this.coeff_in_paillier.get(i).multiply(res).mod(this.nSquared);
            } else if (i == this.coeff_in_paillier.size() - 1) {
                res = this.coeff_in_paillier.get(i).modPow(this.x, this.nSquared);
            } else {
                res = res.multiply(this.coeff_in_paillier.get(i)).modPow(this.x, this.nSquared);
            }
        }
        return res;
    }

    public void genCircuit(){
        BigInteger paillierModulusValue = this.nSquared;
        int paillierModulusSize = paillierModulusValue.bitLength();


        CircuitGenerator generator = new CircuitGenerator("Horner_polynomial_eval_big_int_paillier_gadget_input_file") {
            ArrayList<Wire[]> inputMessageA;

            Wire[] cipherText;
            LongElement paillierModulus;

            HornerPolynomialEvalPaillier hornerPolynomialEvalPaillier;

            @Override
            protected void buildCircuit() {
                paillierModulus = createLongElementInput(paillierModulusSize);
                inputMessageA = new ArrayList<>();
                for(int j = 0; j < this.coeff_in_paillier.size(); j++){
                    Wire[] coefficientWire = createProverWitnessWireArray(this.coeff_in_paillier.get(j).toByteArray().length);
                    for(int i = 0; i < this.coeff_in_paillier.get(j).toByteArray().length; i++){
                        coefficientWire[i].restrictBitLength(8);
                    }
                    inputMessageA.add(coefficientWire);
                }
                
                
                hornerPolynomialEvalPaillier = new HornerPolynomialEvalPaillier(paillierModulus, inputMessageA, this.x, paillierModulusSize);
                
                Wire[] cipherTextInBytes = hornerPolynomialEvalPaillier.getOutputWires(); // in bytes
                
                // group every 8 bytes together
                cipherText = new WireArray(cipherTextInBytes).packWordsIntoLargerWords(8, 8);
                makeOutputArray(cipherText,
                        "Output cipher text");
            }

            @Override
            public void generateSampleInput(CircuitEvaluator evaluator) {
                for(int j = 0; j < this.coeff_in_paillier.size(); j++){
                    byte[] array = this.coeff_in_paillier.get(j).toByteArray();
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
        generator.prepFiles();
        CircuitEvaluator evaluator = generator.getCircuitEvaluator();
        ArrayList<Wire> cipherTextList = generator.getOutWires();
        BigInteger t = BigInteger.ZERO;
        int i = 0;
        for(Wire w:cipherTextList){
            BigInteger val = evaluator.getWireValue(w);
            t = t.add(val.shiftLeft(i*64));
            i++;
        }
        
        System.out.println("Result of our polynomial is ");
        System.out.println(t);
    }

    public static void main(String args[]){
        //Evaluation of the time computation
        long startTime = System.nanoTime();

        HornerPolynomialPaillierFromFileGenerator hornerPolynomialPaillierFromFileGenerator = new HornerPolynomialPaillierFromFileGenerator();
        hornerPolynomialPaillierFromFileGenerator.genCircuit();
        long endTime = System.nanoTime();

        long duration = (endTime - startTime);
        System.out.println("Time of calculation : " + duration/1000000 + " ms");
    }
}    
