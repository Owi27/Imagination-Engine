#pragma once

class ImgnInput
{

public:
	ImgnInput() = default;

	ImgnInput(GWindow& pWin) /*Constructor*/
	{
		bufferedInput.Create(pWin);
	}

	~ImgnInput() /*Destructor*/
	{
	}

	GBufferedInput bufferedInput;
};