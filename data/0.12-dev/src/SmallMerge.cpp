#include "SmallMerge.h"
#include "SetUtils.h"
#include <algorithm> // needed for std::binary_search
#include <iostream> // needed for std::cout
#include <iterator>
using namespace std;

// DEBUGGING OPTIONS: 
//   Enable these to see smallmerge debugging output.
//   DEBUG_SUBSETS will show when complete subsets are merged
//   DEBUG_SMALLMERGE will show when "similar sets" are merged
//   These will be written to standard output, and should NOT 
//   be be used when speed is desired.
#define DEBUG_SUBSETS 0 
#define DEBUG_SMALLMERGE 0


// SMALLMERGE DEFAULTS
//   By default we use a smallmerge ratio of .5, and a pithy
//   filter of 10.  This seems to be about optimal for merging
//   meaning families generated by single-word expansion.
//
double SMALLMERGE_RATIO = .5;
unsigned int PITHY_FILTER = 10;
unsigned int SUBSETS = 0;
unsigned int MERGES = 0;


//
// printvec is a debug helper
//
template<class T>
static void printvec(const vector<T>& v, const char* label)
{
    cout << label << " = { ";
    copy(v.begin(), v.end(), ostream_iterator<T>(cout, ", "));
    cout << "}\n";
}   
    

void smallMerge(MeaningFamily& classes, MeaningFamily& output)
{
    SUBSETS = 0;
    MERGES = 0;
    
    // First we take an initial sweep through, eliminating trivial
    // classes and making sure our input is sorted.
    for(unsigned int i = 0;i < classes.size();++i)
    {
        // Destroy any class with only two words.  These tend to be
        // the cause of extremely obscure synonyms.
        if (classes[i].size() == 2)
            classes[i].clear();
   
        // Ensure that all of our input classes are already sorted
        // so that set_comparison will work on them.
        sort(classes[i].begin(), classes[i].end());
    }
   
     
    for(unsigned int i = 0;i < classes.size();++i)
    {
        // skip over empty classes immediately.  this lets us mark
        // a class as being merged by simply clearing it out.
        if (classes[i].empty())
            continue;
        
        
        for(unsigned int j = i+1;j < classes.size();++j)
        {
            // again, skip over empty classes, we don't need to waste
            // our time with things that have already been dealt with.
            if (classes[j].empty())
                continue;
            
            // do our set comparison to see what kind of sets we've 
            // encountered.  
            vector<string> left, right, common;
    
            set_comparison(
                    left, right, common, 
                    classes[i].begin(), classes[i].end(), 
                    classes[j].begin(), classes[j].end()
            );
            
            // Now we will delete proper subsets. There shouldn't
            // be many of these.
            if (right.empty()) // classes[j] is a subset of classes[i].
            {
                #if DEBUG_SUBSETS
                cout << j << " is a subset of " << i << endl;
                printvec(classes[i], "  lhs");
                printvec(classes[j], "  rhs");
                #endif

                // We want to clear out classes[j] since it is a subset.
                // We are then done processing this set, but since i didn't
                // change we can go ahead and keep processing j's that are
                // farther down the line. (continue)
                SUBSETS++;
                classes[j].clear();
                continue;
            }
            
            else if (left.empty()) // classes[i] is a subset of classes[j].
            {
                #if DEBUG_SUBSETS
                cout << i << " is a subset of " << j << endl;
                printvec(classes[i], "  lhs");
                printvec(classes[j], "  rhs");
                #endif // DEBUG_SUBSETS             
                
                // Since classes[i] is in j, we can just clear it out
                // to merge the two classes.  In addition, we no longer
                // need to be processing classes[i], so break out of our
                // inner loop and go to the next i. (break)
                SUBSETS++;
                classes[i].clear();
                break;
            }

            
            // Now we will attempt to merge small sets into large sets.
            //
            // We will make the convention that merges will be placed
            // into classes[i].  In other words, we want classes[i] to 
            // become our large set, and clear out classes[j].  
            //

            // compute lratio: percent of elements l's elements that
            // are also elements of r, and rratio: percent of r's 
            // elements that are also elements of l.
            double lratio = double(common.size()) / double(common.size() + left.size());
            double rratio = double(common.size()) / double(common.size() + right.size());
                
           
    #if DEBUG_SMALLMERGE 
            
            if (lratio >= SMALLMERGE_RATIO) // lratio is large enough to merge.
            {
                cout << "smallmerge: " << i << " into " << j << " (ratio=" 
                     << lratio << ")" << endl;
                printvec(classes[i], "  lhs");
                printvec(classes[j], "  rhs");
            }

            else if (rratio >= SMALLMERGE_RATIO) // rratio is large enough to merge.
            {
                cout << "smallmerge: " << j << " into " << i << " (ratio=" 
                    << rratio << ")" << endl;
                printvec(classes[i], "  lhs");
                printvec(classes[j], "  rhs");
            }   

    #endif // DEBUG_SMALLMERGE 
            
            if ((rratio >= SMALLMERGE_RATIO) || (lratio >= SMALLMERGE_RATIO))
            {   
                MERGES++;
                
                for(unsigned int k = 0;k < classes[j].size();++k)
                {
                    const string& word = (classes[j])[k];
                    if (!binary_search(classes[i].begin(), classes[i].end(), word))
                    {
                        classes[i].push_back(word);
                        sort(classes[i].begin(), classes[i].end());
                    }
                }
                
                classes[j].clear();
                
                #if DEBUG_SMALLMERGE
                cout << "Post merge setup: " << endl;
                printvec(classes[i], "  lhs");
                printvec(classes[j], "  rhs");
                #endif // DEBUG_SMALLMERGE

                --i;
                break;
            }
        }
    }


    // Finally, eliminate pithy classes and generate our output family.
    for(unsigned int i = 0;i < classes.size();++i)
    {
        if (classes[i].size() > PITHY_FILTER)
        {
            output.push_back(classes[i]);
        }
    }
}

