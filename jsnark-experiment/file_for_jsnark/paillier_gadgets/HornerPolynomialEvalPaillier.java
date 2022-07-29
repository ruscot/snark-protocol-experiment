package examples.gadgets.paillier;

import java.math.BigInteger;
import java.util.ArrayList;

import circuit.auxiliary.LongElement;
import circuit.operations.Gadget;
import circuit.structure.Wire;
import circuit.structure.WireArray;

public class HornerPolynomialEvalPaillier extends Gadget {
    // every wire represents a byte in the following three arrays
    /**
     * coefficients[0] = a_0
     * coefficients[n] = a_n
     * with f(x) = a_0 + ... + a_n * x^n
     * */ 
	private ArrayList<Wire[]> coefficients;
	private BigInteger x;
	private Wire[] result;
	
	private LongElement paillierModulus;
	private int paillierKeyBitLength;

	public HornerPolynomialEvalPaillier(LongElement paillierModulus, ArrayList<Wire[]> coefficients, BigInteger x, int paillierKeyBitLength, String... desc) {
		super(desc);
		this.paillierModulus = paillierModulus;
		this.paillierKeyBitLength = paillierKeyBitLength;
		this.coefficients = coefficients;
        this.x = x;
		buildCircuit();
	}

	private void buildCircuit() {
		LongElement res = new LongElement(
			new BigInteger[] { BigInteger.ONE });
        Wire [] last_operation_wire = this.coefficients.get(0);
        for(int i = this.coefficients.size()-1; i >= 0; i--){
            if (i == 0){
                if (i == this.coefficients.size() -1 ){
                    //We have just one coef, not a polynomial
                    WireArray allBitsA = new WireArray(this.coefficients.get(0)).getBits(8);
	 	            res = new LongElement(allBitsA);
                } else {
                    Wire[] currResMultiplication = new MultiplicationLongElementPaillier(this.paillierModulus, 
                        this.coefficients.get(0), last_operation_wire, this.paillierKeyBitLength, "Multiplication").getOutputWires();
                    WireArray allBitsA = new WireArray(currResMultiplication).getBits(8);
	 	            res = new LongElement(allBitsA);
                }
            } else if(i == this.coefficients.size()-1){
                Wire[] currResModPow = new ModPowLongElementPaillier(this.paillierModulus, this.coefficients.get(i), this.x, 
                                            this.paillierKeyBitLength, "Mod pow operation").getOutputWires();
                last_operation_wire = currResModPow;
            } else {
                Wire[] currResMultiplication = new MultiplicationLongElementPaillier(this.paillierModulus, 
                    this.coefficients.get(i), last_operation_wire, this.paillierKeyBitLength, "Multiplication").getOutputWires();
                Wire[] currResModPow = new ModPowLongElementPaillier(this.paillierModulus, currResMultiplication, this.x, 
                                this.paillierKeyBitLength, "Mod pow operation").getOutputWires();
                last_operation_wire = currResModPow;
            }
        }
		
		this.result = res.getBits(-1).packBitsIntoWords(8);
	}
	
	@Override
	public Wire[] getOutputWires() {
		return this.result;
	}
}

