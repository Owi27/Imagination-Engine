#pragma once

namespace Debug
{
	using namespace std;

	void CheckInputResources(FrameGraph& fg, FrameGraphNode node)
	{
		/*for (auto i : node.inputResources)
		{
			if (auto r = fg.GetBufferResource(i))
			{
				if (r->prepared)
				{
					cout << "Input Resource: " << r->name << " is Prepared\n";
				}
				else
				{
					cout << r->name << " was not prepared. Please execute " << r->parent << " first\n";
				}
			}
		}*/
	}
}