package service

import (
	"github.com/hoop33/entrevista"
	"github.com/hoop33/limo/model"
	"github.com/xanzy/go-gitlab"
)

// Gitlab represents the Gitlab service
type Gitlab struct {
}

// Login logs in to Gitlab
func (g *Gitlab) Login() (string, error) {
	interview := createInterview()
	interview.Questions = []entrevista.Question{
		{
			Key:      "token",
			Text:     "Enter your GitLab API token",
			Required: true,
			Hidden:   true,
		},
	}

	answers, err := interview.Run()
	if err != nil {
		return "", err
	}
	return answers["token"].(string), nil
}

// GetStars returns the stars for the specified user (empty string for authenticated user)
func (g *Gitlab) GetStars(starChan chan<- *model.StarResult, token string, user string) {
	client := g.getClient(token)

	currentPage := 1
	lastPage := 1

	for currentPage <= lastPage {
		projects, response, err := client.Projects.ListStarredProjects(&gitlab.ListProjectsOptions{
			ListOptions: gitlab.ListOptions{
				Page: currentPage,
			},
		})
		// If we got an error, put it on the channel
		if err != nil {
			starChan <- &model.StarResult{
				Error: err,
				Star:  nil,
			}
		} else {
			// Set last page only if we didn't get an error
			lastPage = response.LastPage

			// Create a Star for each repository and put it on the channel
			for _, project := range projects {
				star, err := model.NewStarFromGitlab(*project)
				starChan <- &model.StarResult{
					Error: err,
					Star:  star,
				}
			}
		}
		// Go to the next page
		currentPage++
	}

	close(starChan)
}

func (g *Gitlab) getClient(token string) *gitlab.Client {
	return gitlab.NewClient(nil, token)
}

func init() {
	registerService(&Gitlab{})
}